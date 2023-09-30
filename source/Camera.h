#pragma once
#include <cassert>
#include <SDL_keyboard.h>
#include <SDL_mouse.h>
#include <iostream>

#include "Math.h"
#include "Timer.h"

namespace dae
{
	struct Camera
	{
		Camera() 
		{
			fovScale = tan(TO_RADIANS * fovAngle / 2);
		};

		Camera(const Vector3& _origin, float _fovAngle):
			origin{_origin},
			fovAngle{_fovAngle}
		{
		}


		Vector3 origin{};
		float fovAngle{90.f};
		float fovScale{0.f};

		Vector3 forward{ Vector3::UnitZ };
		Vector3 up{Vector3::UnitY};
		Vector3 right{Vector3::UnitX};

		float totalPitch{0.f};
		float totalYaw{0.f};

		float translateSpeed{ 20.f };

		Matrix cameraToWorld{};


		Matrix CalculateCameraToWorld()
		{
			//todo: W2
			
			right = Vector3::Cross(Vector3::UnitY, forward).Normalized();
			up = Vector3::Cross(forward, right).Normalized();

			return 
			{
				Matrix
				{
					{right, 0},
					{up, 0},
					{forward, 0},
					{origin, 1}
				}
			};
		}

		void Update(Timer* pTimer)
		{
			const float deltaTime = pTimer->GetElapsed();

			//Keyboard Input
			const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);


			//Mouse Input
			int mouseX{}, mouseY{};
			const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

			//todo: W2
			HandleKeyMovement(pKeyboardState, deltaTime);

			if (mouseState == SDL_BUTTON(1))
			{
				if (mouseY != 0)
				{
					totalPitch += mouseY / 2;
					Matrix finalRotation{ Matrix::CreateRotation(totalPitch * TO_RADIANS, totalYaw * TO_RADIANS, 0) };

					forward = finalRotation.TransformVector(Vector3::UnitZ);
					forward.Normalize();
					cameraToWorld = CalculateCameraToWorld();
				}
				if (mouseX != 0)
				{
					totalYaw += mouseX / 2;
					Matrix finalRotation{ Matrix::CreateRotation(totalPitch * TO_RADIANS, totalYaw * TO_RADIANS, 0) };

					forward = finalRotation.TransformVector(Vector3::UnitZ);
					forward.Normalize();
					cameraToWorld = CalculateCameraToWorld();
				}
			}
			
		}

		void HandleKeyMovement(const uint8_t* keys, float elapsedSec)
		{
			if (keys[SDL_SCANCODE_W])
			{
				origin += forward * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_S])
			{
				origin -= forward * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_D])
			{
				origin += right * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_A])
			{
				origin -= right * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_E])
			{
				origin.y += translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_Q])
			{
				origin.y -= translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_UP])
			{
				fovAngle += translateSpeed * elapsedSec;
				if (fovAngle > 360) fovAngle = 360;
			    fovScale = tan(TO_RADIANS * fovAngle / 2);
			}
			if (keys[SDL_SCANCODE_DOWN])
			{
				fovAngle -= translateSpeed * elapsedSec;
				if (fovAngle < 0) fovAngle = 0;
			    fovScale = tan(TO_RADIANS * fovAngle / 2);
			}
		}
	};
}
