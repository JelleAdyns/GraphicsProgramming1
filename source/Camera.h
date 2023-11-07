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
		Camera() = default;

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

		float translateSpeed{ 10.f };

		Matrix cameraToWorld{};

		//Sphere* cameraSphere{};


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

			if (mouseState == SDL_BUTTON(SDL_BUTTON_RIGHT))
			{
				if (mouseY != 0)
				{
					totalPitch += mouseY;
					if (totalPitch >= 89) totalPitch = 89;
					if (totalPitch <= -89) totalPitch = -89;
					TransformForwardVector();
				}
				if (mouseX != 0)
				{
					totalYaw += mouseX;
					TransformForwardVector();
				}
			}
			
			if(mouseState == SDL_BUTTON(SDL_BUTTON_LEFT))
			{
				if (mouseY != 0) origin += forward * float(-mouseY) * translateSpeed * deltaTime;

				if (mouseX != 0)
				{
					totalYaw += mouseX;
					TransformForwardVector();
				}
			}
			if (mouseState == (SDL_BUTTON(SDL_BUTTON_LEFT) | SDL_BUTTON(SDL_BUTTON_RIGHT)))
			{
				if (mouseY != 0) origin.y += mouseY * translateSpeed * deltaTime;
			}
			
		}

		void HandleKeyMovement(const uint8_t* keys, float elapsedSec)
		{
			if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])
			{
				origin += forward * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])
			{
				origin -= forward * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT])
			{
				origin += right * translateSpeed * elapsedSec;
			}
			if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])
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
			if (keys[SDL_SCANCODE_Z])
			{
				fovAngle += translateSpeed * elapsedSec;
				if (fovAngle > 360) fovAngle = 360;
			    fovScale = tan(TO_RADIANS * fovAngle / 2);
			}
			if (keys[SDL_SCANCODE_C])
			{
				fovAngle -= translateSpeed * elapsedSec;
				if (fovAngle < 0) fovAngle = 0;
			    fovScale = tan(TO_RADIANS * fovAngle / 2);
			}
			//cameraSphere->origin = origin;
		}

		void TransformForwardVector()
		{
			Matrix finalRotation{ Matrix::CreateRotation(totalPitch * TO_RADIANS, totalYaw * TO_RADIANS, 0) };

			forward = finalRotation.TransformVector(Vector3::UnitZ).Normalized();
			cameraToWorld = CalculateCameraToWorld();
		}
	};
}
