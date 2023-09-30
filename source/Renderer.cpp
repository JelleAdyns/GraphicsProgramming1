//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window * pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	float ascpectRatio{ float(m_Width) / m_Height };
	
	float z{ 1.f };

	for (int px{}; px < m_Width; ++px)
	{
		float x{ (2 * (px + 0.5f) / m_Width - 1) * ascpectRatio * camera.fovScale};

		for (int py{}; py < m_Height; ++py)
		{
			float y{ (1 - 2 * (py + 0.5f) / m_Height) * camera.fovScale };
			
			Vector3 direction{ (camera.cameraToWorld.TransformVector({ x,y,z })) };

			direction.Normalize();

			Ray ray{ camera.origin, direction };

			ColorRGB finalColor{};
			HitRecord closestHit{};

			pScene->GetClosestHit(ray, closestHit);
			if (closestHit.didHit) finalColor = materials[closestHit.materialIndex]->Shade();

			Ray lightRay{ };
			lightRay.origin = closestHit.origin + closestHit.normal * 0.001f;

			for (const auto& light : lights)
			{
				Vector3 hitToLight{ LightUtils::GetDirectionToLight(light, lightRay.origin) };

				lightRay.direction = hitToLight.Normalized();
				lightRay.max = hitToLight.Magnitude();

				if (pScene->DoesHit(lightRay))
				{
					finalColor /= 2;
				}
			}
			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}
