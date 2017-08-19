#ifndef MAINEVENTRECEIVER_H_INCLUDE
#define MAINEVENTRECEIVER_H_INCLUDE

#include <irrlicht.h>
#include <string>

struct Input {
    public:
        enum class DEVICE {
            KEY,
            MOUSE
        };

        DEVICE device;

       irr::EKEY_CODE keyCode;
       unsigned char mouseButton;

        Input(DEVICE inDevice,irr::EKEY_CODE inCode);
        Input(DEVICE inDevice,unsigned char inCode);

        bool operator<(const Input& other) const;
    private:
        Input();
};

class MainEventReceiver : public irr::IEventReceiver {
    private:
		bool keyDown[irr::KEY_KEY_CODES_COUNT];
		int keyHit[irr::KEY_KEY_CODES_COUNT];
		bool mouseDown[3];
		int mouseHit[3];

		irr::EKEY_CODE lastKeyHit = irr::KEY_KEY_CODES_COUNT;

        irr::core::stringw charQueue;

        irr::core::vector2di MousePosition;
        float MouseWheel;

        bool errorState;
        std::string errorStr;
    public:
        virtual bool OnEvent(const irr::SEvent& event);

        virtual irr::core::stringw getCharQueue(irr::core::stringw in=L"");
        virtual void clearCharQueue();

        virtual bool getKeyDown(irr::EKEY_CODE keyCode) const;
        virtual int getKeyHit(irr::EKEY_CODE keyCode);
		virtual irr::EKEY_CODE getLastKeyHit();
        virtual bool getMouseDown(unsigned char keyCode) const;
        virtual int getMouseHit(unsigned char keyCode);
        virtual bool getInput(const Input& input) const;
        virtual irr::core::position2di getMousePos() const;

        virtual float getMouseWheel();

		void clearMouse();
		void clearKeys();

        bool getErrorState();

        std::string getErrorStr();

        void resetErrorState();

        MainEventReceiver();
};

#endif
