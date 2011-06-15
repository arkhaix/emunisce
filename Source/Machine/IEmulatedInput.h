#ifndef IEMULATEDINPUT_H
#define IEMULATEDINPUT_H

namespace Buttons
{
	typedef int Type;

	enum
	{
		Right = 0,
		Left,
		Up,
		Down,

		A,
		B,
		Select,
		Start,

		NumButtons
	};

	static const char* ToString[] =
	{
		"Right",
		"Left",
		"Up",
		"Down",

		"A",
		"B",
		"Select",
		"Start",

		"NumButtons"
	};

}	//namespace Buttons


class IEmulatedInput
{
public:

	virtual void ButtonDown(Buttons::Type button) = 0;
	virtual void ButtonUp(Buttons::Type button) = 0;
};

#endif
