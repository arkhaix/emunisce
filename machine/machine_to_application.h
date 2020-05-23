#ifndef IMACHINETOAPPLICATION_H
#define IMACHINETOAPPLICATION_H

namespace emunisce {

class MachineToApplication {
public:
	virtual void HandleApplicationEvent(unsigned int eventId) = 0;  ///< For application-requested mid-frame interrupts

	virtual void SaveRomData(const char* title, unsigned char* buffer, unsigned int bytes) = 0;

	virtual unsigned int GetRomDataSize(const char* title) = 0;
	virtual void LoadRomData(const char* title, unsigned char* buffer, unsigned int bytes) = 0;
};

}  // namespace emunisce

#endif
