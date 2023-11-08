//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"
#include <execution>

#define PARALLEL_EXECUTION
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
	const Camera& camera = pScene->GetCamera();
	const auto& materials{pScene->GetMaterials()};
	const auto& lights{ pScene->GetLights() };
	const float& aspectRatio{ float(m_Width) / m_Height };

#if defined (PARALLEL_EXECUTION)
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };
	std::vector<uint32_t> pixelIndices{};
	pixelIndices.reserve(amountOfPixels);
	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; ++pixelIndex) pixelIndices.emplace_back(pixelIndex);
	
	std::for_each(std::execution::par, pixelIndices.begin(), pixelIndices.end(), [&](int i)
		{
			RenderPixel(pScene, i, aspectRatio, camera, materials, lights);
		});
		
	
#else
	uint32_t amountOfPixels{ uint32_t(m_Width * m_Height) };

	for (uint32_t pixelIndex{}; pixelIndex < amountOfPixels; ++pixelIndex)
	{
		RenderPixel(pScene, i, aspectRatio, camera, materials, lights);
	}
#endif
	

	//@END
	//Update SDL Surface;
	SDL_UpdateWindowSurface(m_pWindow);
}
void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float aspectRatio, const Camera& camera, const std::vector<dae::Material*>& materials, const std::vector<dae::Light>& lights) const
{
	const uint32_t px{ pixelIndex % m_Width }, py{ pixelIndex / m_Width };

	const float x{ (2 * (px + 0.5f) / m_Width - 1) * aspectRatio * camera.fovScale };
	const float y{ (1 - 2 * (py + 0.5f) / m_Height) * camera.fovScale };
	const float z{ 1.f };

	Vector3 direction{ (camera.cameraToWorld.TransformVector({ x,y,z })) };

	direction.Normalize();

	const Ray ray{ camera.origin, direction };

	ColorRGB finalColor{};
	HitRecord closestHit{};

	pScene->GetClosestHit(ray, closestHit);
	if (closestHit.didHit)
	{
		Ray lightRay{ };
		lightRay.origin = closestHit.origin + closestHit.normal * 0.001f;

		for (int i = 0; i < lights.size(); ++i)
		{
			lightRay.direction = LightUtils::GetDirectionToLight(lights[i], lightRay.origin);
			if (lights[i].type == LightType::Directional)
			{
				lightRay.direction.Normalize();
				lightRay.max = FLT_MAX;
			}
			else lightRay.max = lightRay.direction.Normalize();


			if (m_ShadowsEnabled)
			{
				if (pScene->DoesHit(lightRay)) continue;
			}

			switch (m_CurrentLightingMode)
			{
			case LightingMode::ObservedArea:
			{
				const float cosTheta = Vector3::Dot(closestHit.normal, lightRay.direction);
				if (cosTheta <= 0) continue;
				finalColor.r += cosTheta;
				finalColor.g += cosTheta;
				finalColor.b += cosTheta;
				break;
			}
			case LightingMode::Radiance:
			{
				finalColor += LightUtils::GetRadiance(lights[i], lightRay.origin);
				break;
			}
			case LightingMode::BRDF:
			{
				finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, -direction);
				break;
			}
			case LightingMode::Combined:
			{
				const float cosTheta = Vector3::Dot(closestHit.normal, lightRay.direction);
				if (cosTheta <= 0) continue;
				ColorRGB radiance{ LightUtils::GetRadiance(lights[i], lightRay.origin) };
				ColorRGB shade{ materials[closestHit.materialIndex]->Shade(closestHit, lightRay.direction, -direction) };
				finalColor += radiance * shade * cosTheta;
				break;
			}
			}
		}
	}
	finalColor.MaxToOne();

	m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
		static_cast<uint8_t>(finalColor.r * 255),
		static_cast<uint8_t>(finalColor.g * 255),
		static_cast<uint8_t>(finalColor.b * 255));

	
}
bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:
		m_CurrentLightingMode = LightingMode::Radiance;
		std::cout << "RADIANCE ONLY" << std::endl;
		break;
	case LightingMode::Radiance:
		m_CurrentLightingMode = LightingMode::BRDF;
		std::cout << "BRDF ONLY" << std::endl;
		break;
	case LightingMode::BRDF:
		m_CurrentLightingMode = LightingMode::Combined;
		std::cout << "COMBINED" << std::endl;
		break;
	case LightingMode::Combined:
		m_CurrentLightingMode = LightingMode::ObservedArea;
		std::cout << "OBSERVED AREA ONLY" << std::endl;
		break;
	}
}