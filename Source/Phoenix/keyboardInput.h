#ifndef KEYBOARDINPUT_H
#define KEYBOARDINPUT_H

class Phoenix;

class Machine;
class Input;

class KeyboardInput
{
public:

	KeyboardInput();

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(Machine* machine);

	void KeyDown(int key);
	void KeyUp(int key);

private:

	class KeyboardInput_Private* m_private;

};




#endif
