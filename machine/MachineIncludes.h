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

// This is a convenience header for pulling in all Machine types
// This only includes the abstract types.
// Include the specific system for concrete types.

#include "emulated_display.h"
#include "emulated_input.h"
#include "emulated_machine.h"
#include "emulated_memory.h"
#include "emulated_processor.h"
#include "emulated_sound.h"
#include "machine_to_application.h"
#include "machine_factory.h"
#include "machine_serialization.h"
#include "machine_types.h"
#include "screen_buffer.h"
