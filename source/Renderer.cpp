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
	float fovScale{ tan(TO_RADIANS * camera.fovAngle) };

	float z{ 1.f };

	Vector3 direction{};
	Ray ray{ camera.origin, direction };


	for (int px{}; px < m_Width; ++px)
	{
		float x{ (2 * (px + 0.5f) / m_Width - 1) * ascpectRatio * fovScale};

		for (int py{}; py < m_Height; ++py)
		{
	
			float y{ (1 - 2 * (py + 0.5f) / m_Height)*fovScale };
			
			direction.x = x;
			direction.y = y;
			direction.z = z;

			direction = (camera.cameraToWorld.TransformVector(direction)) ;

			direction.Normalize();
			ray.direction = direction;
			ColorRGB finalColor{ };
			HitRecord closestHit{};

			pScene->GetClosestHit(ray, closestHit);
			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				/*float scale_t = (closestHit.t - 50.f) / 40.f;
				finalColor = { scale_t,scale_t,scale_t };*/
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
