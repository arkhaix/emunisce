#ifndef IMACHINETOAPPLICATION_H
#define IMACHINETOAPPLICATION_H

namespace Emunisce
{

class IMachineToApplication
{
public:

	virtual void SaveRomData(const char* title, unsigned char* buffer, unsigned int bytes) = 0;

	virtual unsigned int GetRomDataSize(const char* title) = 0;
	virtual void LoadRomData(const char* title, unsigned char* buffer, unsigned int bytes) = 0;
};

}	//namespace Emunisce

#endif
