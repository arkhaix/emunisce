/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "sdl_gpu_renderer.h"

#include <SDL3/SDL.h>

#include <cstdlib>
#include <cstring>

#include "machine_includes.h"
#include "platform_includes.h"

namespace emunisce {

class SDLGPURenderer_Private {
public:
	EmulatedMachine* _Machine;
	EmulatedDisplay* _Display;

	SDL_Window* _Window;
	SDL_GPUDevice* _Device;
	SDL_GPUTexture* _ScreenTexture;
	SDL_GPUTransferBuffer* _TransferBuffer;
	
	int _LastFrameRendered;
	int _LastFrameDrawn;

	int _ClientWidth;
	int _ClientHeight;

	int _TextureWidth;
	int _TextureHeight;

	bool _VsyncEnabled;

	SDLGPURenderer_Private() {
		_Machine = nullptr;
		_Display = nullptr;
		_Window = nullptr;
		_Device = nullptr;
		_ScreenTexture = nullptr;
		_TransferBuffer = nullptr;

		_LastFrameRendered = -1;
		_LastFrameDrawn = -1;

		_ClientWidth = 1;
		_ClientHeight = 1;

		_TextureWidth = 0;
		_TextureHeight = 0;

		_VsyncEnabled = true;
	}

	void InitializeGPU(SDL_Window* window) {
		_Window = window;

		SDL_Log("InitializeGPU: Starting GPU initialization...");

		// Create GPU device
		_Device = SDL_CreateGPUDevice(
			SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_MSL,
			true,  // debugMode
			nullptr  // name
		);

		if (!_Device) {
			SDL_Log("Failed to create GPU device: %s", SDL_GetError());
			return;
		}

		SDL_Log("InitializeGPU: GPU device created successfully");

		// Claim the window
		if (!SDL_ClaimWindowForGPUDevice(_Device, window)) {
			SDL_Log("Failed to claim window for GPU device: %s", SDL_GetError());
			SDL_DestroyGPUDevice(_Device);
			_Device = nullptr;
			return;
		}

		SDL_Log("InitializeGPU: Window claimed successfully");

		// Get initial window size
		SDL_GetWindowSize(_Window, &_ClientWidth, &_ClientHeight);
	}

	void ShutdownGPU() {
		if (_TransferBuffer) {
			SDL_ReleaseGPUTransferBuffer(_Device, _TransferBuffer);
			_TransferBuffer = nullptr;
		}

		if (_ScreenTexture) {
			SDL_ReleaseGPUTexture(_Device, _ScreenTexture);
			_ScreenTexture = nullptr;
		}

		if (_Device) {
			SDL_ReleaseWindowFromGPUDevice(_Device, _Window);
			SDL_DestroyGPUDevice(_Device);
			_Device = nullptr;
		}
	}

	void SetVsync(bool enabled) {
		_VsyncEnabled = enabled;
	}

	void RecreateTexture(int width, int height) {
		// Release old resources
		if (_TransferBuffer) {
			SDL_ReleaseGPUTransferBuffer(_Device, _TransferBuffer);
			_TransferBuffer = nullptr;
		}

		if (_ScreenTexture) {
			SDL_ReleaseGPUTexture(_Device, _ScreenTexture);
			_ScreenTexture = nullptr;
		}

		_TextureWidth = width;
		_TextureHeight = height;

		// Create new texture
		SDL_GPUTextureCreateInfo textureInfo = {};
		textureInfo.type = SDL_GPU_TEXTURETYPE_2D;
		textureInfo.format = SDL_GPU_TEXTUREFORMAT_B8G8R8A8_UNORM;  // BGRA format matching DisplayPixel
		textureInfo.width = width;
		textureInfo.height = height;
		textureInfo.layer_count_or_depth = 1;
		textureInfo.num_levels = 1;
		textureInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
		textureInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;

		_ScreenTexture = SDL_CreateGPUTexture(_Device, &textureInfo);
		if (!_ScreenTexture) {
			SDL_Log("Failed to create GPU texture: %s", SDL_GetError());
			return;
		}

		// Create transfer buffer for uploading texture data
		SDL_GPUTransferBufferCreateInfo transferInfo = {};
		transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
		transferInfo.size = width * height * sizeof(DisplayPixel);

		_TransferBuffer = SDL_CreateGPUTransferBuffer(_Device, &transferInfo);
		if (!_TransferBuffer) {
			SDL_Log("Failed to create GPU transfer buffer: %s", SDL_GetError());
		}
	}

	void UpdateTexture(EmulatedMachine* machine) {
		static int updateCounter = 0;

		if (machine == nullptr || _Device == nullptr) {
			if (updateCounter++ % 120 == 0) {
				SDL_Log("UpdateTexture: Missing machine or device");
			}
			return;
		}

		// Get display from machine - it may be wrapped by features
		EmulatedDisplay* display = machine->GetDisplay();
		if (display == nullptr) {
			if (updateCounter++ % 120 == 0) {
				SDL_Log("UpdateTexture: No display available");
			}
			return;
		}

		int currentFrameCount = display->GetScreenBufferCount();
		
		if (updateCounter++ % 120 == 0) {
			SDL_Log("UpdateTexture: machine=%p, display=%p, currentFrameCount=%d, _LastFrameRendered=%d", 
				(void*)machine, (void*)display, currentFrameCount, _LastFrameRendered);
		}

		if (currentFrameCount == _LastFrameRendered) {
			return;
		}

		_LastFrameRendered = currentFrameCount;

		if (updateCounter % 120 == 1) {
			SDL_Log("UpdateTexture: Updating to frame %d", _LastFrameRendered);
		}

		ScreenBuffer* displayScreen = display->GetStableScreenBuffer();

		int displayWidth = displayScreen->GetWidth();
		int displayHeight = displayScreen->GetHeight();

		// Recreate texture if size changed
		if (_ScreenTexture == nullptr || _TextureWidth != displayWidth || _TextureHeight != displayHeight) {
			RecreateTexture(displayWidth, displayHeight);
			if (!_ScreenTexture || !_TransferBuffer) {
				return;
			}
		}

		// We'll do the upload and blit in the same command buffer in RenderToDisplay
		// This is just for tracking that we have new data
	}

	void RenderToDisplay(EmulatedMachine* machine) {
		if (_ScreenTexture == nullptr || _Device == nullptr || _TextureWidth <= 0 || _TextureHeight <= 0) {
			SDL_Log("RenderToDisplay: Missing requirements - texture=%p, device=%p, w=%d, h=%d", 
				_ScreenTexture, _Device, _TextureWidth, _TextureHeight);
			return;
		}

		if (machine == nullptr) {
			SDL_Log("RenderToDisplay: No machine");
			return;
		}

		// Get display from machine - it may be wrapped by features
		EmulatedDisplay* display = machine->GetDisplay();
		if (display == nullptr) {
			SDL_Log("RenderToDisplay: No display");
			return;
		}

		if (_LastFrameDrawn == _LastFrameRendered) {
			// Already rendered this frame
			return;
		}

		_LastFrameDrawn = _LastFrameRendered;

		ScreenBuffer* displayScreen = display->GetStableScreenBuffer();
		if (!displayScreen) {
			SDL_Log("RenderToDisplay: Failed to get stable screen buffer");
			return;
		}

		//SDL_Log("RenderToDisplay: Drawing frame %d, texture size=%dx%d, screen size=%dx%d", 
		//	_LastFrameRendered, _TextureWidth, _TextureHeight, 
		//	displayScreen->GetWidth(), displayScreen->GetHeight());

		// Acquire command buffer
		SDL_GPUCommandBuffer* cmdBuffer = SDL_AcquireGPUCommandBuffer(_Device);
		if (!cmdBuffer) {
			return;
		}

		// First, upload the texture data via a copy pass
		SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuffer);
		if (copyPass) {
			// Map transfer buffer and copy pixel data
			void* transferData = SDL_MapGPUTransferBuffer(_Device, _TransferBuffer, false);
			if (transferData) {
				memcpy(transferData, displayScreen->GetPixels(), _TextureWidth * _TextureHeight * sizeof(DisplayPixel));
				SDL_UnmapGPUTransferBuffer(_Device, _TransferBuffer);

				// Upload from transfer buffer to texture
				SDL_GPUTextureTransferInfo transferInfo = {};
				transferInfo.transfer_buffer = _TransferBuffer;
				transferInfo.offset = 0;

				SDL_GPUTextureRegion textureRegion = {};
				textureRegion.texture = _ScreenTexture;
				textureRegion.mip_level = 0;
				textureRegion.layer = 0;
				textureRegion.x = 0;
				textureRegion.y = 0;
				textureRegion.z = 0;
				textureRegion.w = _TextureWidth;
				textureRegion.h = _TextureHeight;
				textureRegion.d = 1;

				SDL_UploadToGPUTexture(copyPass, &transferInfo, &textureRegion, false);
			}
			SDL_EndGPUCopyPass(copyPass);
		}

		// Acquire swapchain texture
		SDL_GPUTexture* swapchainTexture = nullptr;
		if (!SDL_WaitAndAcquireGPUSwapchainTexture(
				cmdBuffer,
				_Window,
				&swapchainTexture,
				nullptr,
				nullptr)) {
			SDL_SubmitGPUCommandBuffer(cmdBuffer);
			return;
		}

		if (!swapchainTexture) {
			SDL_SubmitGPUCommandBuffer(cmdBuffer);
			return;
		}

		// Use blit to copy texture to swapchain
		SDL_GPUBlitInfo blitInfo = {};
		blitInfo.source.texture = _ScreenTexture;
		blitInfo.source.mip_level = 0;
		blitInfo.source.layer_or_depth_plane = 0;
		blitInfo.source.x = 0;
		blitInfo.source.y = 0;
		blitInfo.source.w = _TextureWidth;
		blitInfo.source.h = _TextureHeight;
		
		blitInfo.destination.texture = swapchainTexture;
		blitInfo.destination.mip_level = 0;
		blitInfo.destination.layer_or_depth_plane = 0;
		blitInfo.destination.x = 0;
		blitInfo.destination.y = 0;
		blitInfo.destination.w = _ClientWidth;
		blitInfo.destination.h = _ClientHeight;
		
		blitInfo.load_op = SDL_GPU_LOADOP_CLEAR;
		blitInfo.clear_color.r = 0.0f;
		blitInfo.clear_color.g = 0.0f;
		blitInfo.clear_color.b = 0.0f;
		blitInfo.clear_color.a = 1.0f;
		blitInfo.filter = SDL_GPU_FILTER_NEAREST;
		blitInfo.flip_mode = SDL_FLIP_NONE;

		SDL_BlitGPUTexture(cmdBuffer, &blitInfo);

		// Submit command buffer
		SDL_SubmitGPUCommandBuffer(cmdBuffer);
	}

	void Draw(EmulatedMachine* machine) {
		static int drawCounter = 0;
		if (drawCounter++ % 120 == 0) {
			SDL_Log("Draw() called, Machine=%p, Device=%p, LastFrameRendered=%d",
				(void*)machine, (void*)_Device, _LastFrameRendered);
		}
		UpdateTexture(machine);
		RenderToDisplay(machine);
	}

	void Resize(int newWidth, int newHeight) {
		_ClientWidth = newWidth;
		_ClientHeight = newHeight;
	}
};

}  // namespace emunisce

using namespace emunisce;

SDLGPURenderer::SDLGPURenderer() {
	m_private = new SDLGPURenderer_Private();
}

SDLGPURenderer::~SDLGPURenderer() {
	delete m_private;
}

void SDLGPURenderer::Initialize(SDL_Window* window) {
	m_private->InitializeGPU(window);
}

void SDLGPURenderer::Shutdown() {
	m_private->ShutdownGPU();
}

void SDLGPURenderer::SetMachine(EmulatedMachine* machine) {
	m_private->_Machine = machine;
	m_private->_Display = machine->GetDisplay();
	SDL_Log("SetMachine: Machine=%p, Display=%p, Device=%p", 
		(void*)machine, (void*)m_private->_Display, (void*)m_private->_Device);
}

int SDLGPURenderer::GetLastFrameRendered() {
	return m_private->_LastFrameRendered;
}

void SDLGPURenderer::SetVsync(bool enabled) {
	m_private->SetVsync(enabled);
}

void SDLGPURenderer::Draw(EmulatedMachine* machine) {
	m_private->Draw(machine);
}

void SDLGPURenderer::Resize(int newWidth, int newHeight) {
	m_private->Resize(newWidth, newHeight);
}
