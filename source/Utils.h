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

			if (t < ray.min) return false;
			if (t > ray.max) return false; 
			if (t > hitRecord.t) return false;
			
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

			if (t < ray.min) return false;
			if (t > ray.max) return false;
			if (t > hitRecord.t) return false;

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

			if (t < ray.min) return false;
			if (t > ray.max) return false;
			if (t > hitRecord.t) return false;

			const Vector3 P{ ray.origin + ray.direction * t };

			//Won 12 frames with this
			Vector3 e{ triangle.v1 - triangle.v0 };
			Vector3 p{ P - triangle.v0 };
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) <= 0) return false;

			e = triangle.v2 - triangle.v1;
			p = P - triangle.v1;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) <= 0) return false;

			e = triangle.v0 - triangle.v2;
			p = P - triangle.v2;
			if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) <= 0) return false;

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
			bool hit{};
			HitRecord tempHitRec{};
			Triangle triangle{};
			for (int i = 0; i < mesh.normals.size(); ++i)
			{
				triangle.v0 = mesh.transformedPositions[mesh.indices[i * 3]];
				triangle.v1 = mesh.transformedPositions[mesh.indices[i * 3 + 1]];
				triangle.v2 = mesh.transformedPositions[mesh.indices[i * 3 + 2]];
				triangle.cullMode = mesh.cullMode;
				triangle.materialIndex = mesh.materialIndex;
				triangle.normal = mesh.transformedNormals[i];


				const float dotValue{ Vector3::Dot(ray.direction, triangle.normal) };
				switch (triangle.cullMode)
				{
				case TriangleCullMode::FrontFaceCulling:
					if (ignoreHitRecord)
					{
						if (dotValue >= 0) continue;
					}
					else if (dotValue <= 0) continue;
					break;
				case TriangleCullMode::BackFaceCulling:
					if (ignoreHitRecord)
					{
						if (dotValue <= 0) continue;
					}
					else if (dotValue >= 0) continue;
					break;
				case TriangleCullMode::NoCulling:
					if (AreEqual(dotValue, 0)) continue;
					break;
				}
				const float t{ Vector3::Dot(triangle.v0 - ray.origin, triangle.normal) / dotValue };

				if (t < ray.min) continue;
				if (t > ray.max) continue;
				if (t > hitRecord.t) continue;

				const Vector3 P{ ray.origin + ray.direction * t };

				//Won 12 frames with this
				Vector3 e{ triangle.v1 - triangle.v0 };
				Vector3 p{ P - triangle.v0 };
				if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) <= 0) continue;

				e = triangle.v2 - triangle.v1;
				p = P - triangle.v1;
				if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) <= 0) continue;

				e = triangle.v0 - triangle.v2;
				p = P - triangle.v2;
				if (Vector3::Dot(Vector3::Cross(e, p), triangle.normal) <= 0) continue;

				if (!ignoreHitRecord)
				{
					hitRecord.t = t;
					hitRecord.didHit = true;
					hitRecord.origin = P;
					hitRecord.normal = triangle.normal;
					hitRecord.materialIndex = triangle.materialIndex;
				}

				hit = true;
			}

			return hit;
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