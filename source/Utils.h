#pragma once
#include <cassert>
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			const Vector3 fromSphereToRayOrigin{  ray.origin - sphere.origin };

			//const float a{ Vector3::Dot(ray.direction, ray.direction) };
			const float b{ Vector3::Dot(ray.direction, fromSphereToRayOrigin) };
			const float c{ Vector3::Dot(fromSphereToRayOrigin, fromSphereToRayOrigin) - sphere.radius * sphere.radius};
			
			const float discriminant{ b * b - c };

			if (discriminant <= 0) return false;
		
			const float squareRoot{ sqrt(discriminant) };

			const float t{ (-b - squareRoot) };

			if (t < ray.min || t > ray.max || t > hitRecord.t) return false;
			
			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = (hitRecord.origin - sphere.origin) / sphere.radius;
				hitRecord.materialIndex = sphere.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W1
			//const Vector3 fromRayToPlaneOrigin{  };
			const float dotValue{ Vector3::Dot(ray.direction, plane.normal) };

			if (dotValue >= 0) return false;

			const float t{ Vector3::Dot(plane.origin - ray.origin, plane.normal) / dotValue };

			if (t < ray.min || t > ray.max || t > hitRecord.t) return false;

			if(!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = plane.normal;
				hitRecord.materialIndex = plane.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			const float dotValue{ Vector3::Dot(ray.direction, triangle.normal) };
			switch (triangle.cullMode)
			{
			case TriangleCullMode::FrontFaceCulling:
				if (ignoreHitRecord)
				{
					if (dotValue >= 0) return false;
				}
				else if (dotValue <= 0) return false;
				break;
			case TriangleCullMode::BackFaceCulling:
				if (ignoreHitRecord)
				{
					if (dotValue <= 0) return false;
				}
				else if (dotValue >= 0) return false;
				break;
			case TriangleCullMode::NoCulling:
				if (AreEqual(dotValue, 0)) return false;
				break;
			}
			const float t{ Vector3::Dot(triangle.v0 - ray.origin, triangle.normal) / dotValue };

			if (t < ray.min || t > ray.max || t > hitRecord.t) return false;

			const Vector3 P{ ray.origin + ray.direction * t };
			
			
			std::vector<Vector3> vecVertices{ triangle.v0, triangle.v1, triangle.v2 };
			for (int i = 0; i < 3; ++i)
			{
				int nextVertex{ i + 1 };
				if (nextVertex == 3) nextVertex = 0;
				Vector3 e{ vecVertices[nextVertex] - vecVertices[i] };
				Vector3 p{ P - vecVertices[i] };
				
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) < 0) return false;
			}


			if (!ignoreHitRecord)
			{
				hitRecord.t = t;
				hitRecord.didHit = true;
				hitRecord.origin = P;
				hitRecord.normal = triangle.normal;
				hitRecord.materialIndex = triangle.materialIndex;
			}

			return true;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			//assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}

	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			//todo W3
			switch (light.type)
			{
			case LightType::Point:
				return light.origin - origin;
				break;
			case LightType::Directional:
				return -light.direction;
				break;
			default:
				return{};
				break;
			}
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			const float rSquared{(light.origin - target).SqrMagnitude()};
			switch (light.type)
			{
			case LightType::Point:
				return { light.intensity / rSquared * light.color };
				break;
			case LightType::Directional:
				return { light.intensity * light.color };
				break;
			default:
				return{};
				break;
			}
			
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back((int)i0 - 1);
					indices.push_back((int)i1 - 1);
					indices.push_back((int)i2 - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof()) 
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if(isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}