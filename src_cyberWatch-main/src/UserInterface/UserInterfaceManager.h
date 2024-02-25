#pragma once

#include "UserInterface/Icons/IconStruct.h"

class UserInterfaceManager {

	public:

		static UserInterfaceManager *getInstance();

		void showSplashScreen();

		void showExitScreen();

		void render();

		bool handleTouch();

		void handlePEKShort();

		bool isSleepForbidden();

		void setToDefaultApp();

		void renderIcon(IconStruct icon, uint8_t x, uint8_t y);

		bool isLocked();
		void setIsLocked(bool locked);

		void clearScreen();

		protected:

			const int8_t VECTOR_LEFT = 1;
			const int8_t VECTOR_RIGHT = 2;
			const int8_t VECTOR_UP = 1;
			const int8_t VECTOR_DOWN = 2;

			uint16_t swipeLastX = 0;
			uint8_t swipeCounterHorizontal = 0;
			int8_t swipeVectorHorizontal = 0;

			uint16_t swipeLastY = 0;
			uint8_t swipeCounterVertical = 0;
			int8_t swipeVectorVertical = 0;

			bool swipeEnabled = false;

			static UserInterfaceManager *inst;

			bool touchReleased = true;
			bool touchFromInactivity = false;;

			uint lastTouched = 0;

			int16_t lastTouchX;
			int16_t lastTouchY;
			bool swipeWasHandled = false;

			uint8_t longtouchPreventinMovingFingerXMin;
			uint8_t longtouchPreventinMovingFingerXMax;
			uint8_t longtouchPreventinMovingFingerYMin;
			uint8_t longtouchPreventinMovingFingerYMax;

			bool locked = true;

			UserInterfaceManager() {
			}

			void stopSwipeHandlerHorizontal();

			void handleSwipeHorizontal(uint x);

			void stopSwipeHandlerVertical();

			void handleSwipeVertical(uint y);

			bool isLongtouchOnSameCoords();
			void setLongtouchOnSameCoords(uint8_t x, uint8_t y);

			void handleTouchReleased();


};
