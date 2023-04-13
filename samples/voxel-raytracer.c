
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>

#define VOXEL_EPSILON 0.0001f

typedef struct VoxelModel VoxelModel;
struct VoxelModel {
	f32x3                 position;
	f32x3                 half_size;
	HccRoTexture3D(f32x4) color;
};

typedef struct VoxelRaytracerBC VoxelRaytracerBC;
struct VoxelRaytracerBC {
	HccRoBuffer(VoxelModel) models;
	HccRwTexture2D(f32x4)   output;
	float                   time_;
	uint32_t                screen_width;
	uint32_t                screen_height;
};

#ifdef __HCC__
#include <hmaths.h>

typedef struct AabbVsRayHit AabbVsRayHit;
struct AabbVsRayHit {
	bool success;
	float tmin;
	float tmax;
};

AabbVsRayHit aabb_vs_ray3d(f32x3 aabb_min, f32x3 aabb_max, f32x3 ray_origin, f32x3 ray_dir) {
	if (ray_dir.x == 0.f) {
		ray_dir.x = VOXEL_EPSILON;
	}
	if (ray_dir.y == 0.f) {
		ray_dir.y = VOXEL_EPSILON;
	}
	if (ray_dir.z == 0.f) {
		ray_dir.z = VOXEL_EPSILON;
	}
	f32x3 inv_ray_dir = sdivG(1.f, ray_dir);

	f32x3 t0 = mulG(subG(aabb_min, ray_origin), inv_ray_dir);
	f32x3 t1 = mulG(subG(aabb_max, ray_origin), inv_ray_dir);

	f32x3 tmin = minG(t0, t1);
	f32x3 tmax = maxG(t0, t1);

	float tmin_elmt = maxelmtG(tmin);
	float tmax_elmt = minelmtG(tmax);

	AabbVsRayHit hit = {
		.success = tmax_elmt >= 0.f && tmax_elmt >= tmin_elmt,
		.tmin = tmin_elmt,
		.tmax = tmax_elmt,
	};

	return hit;
}

f32x3 voxel_ray_direction_for_uv(float fov, f32x2 size, f32x2 uv) {
	f32x2 xy = subG(uv, divsG(size, 2.f));
	float z = size.y / tanG(radiansG(fov) / 2.f);
	return normG(f32x3(xy.x, -xy.y, z));
}

f32x2x2 voxel_mat2_identity_rotation(float angle) {
	float ca = cosG(angle);
	float sa = sinG(angle);
	f32x2x2 m;
	m.cols[0].x = ca;
	m.cols[0].y = -sa;
	m.cols[1].x = sa;
	m.cols[1].y = ca;
	return m;
}

HCC_COMPUTE(8, 8, 1)
void voxel_raytracer_cs(HccComputeSV const* const sv, VoxelRaytracerBC const* const bc) {
	f32x2 screen_size = f32x2(bc->screen_width, bc->screen_height);
	f32x2 coord = f32x2(sv->dispatch_idx.x, sv->dispatch_idx.y);
	f32x3 ray_dir = voxel_ray_direction_for_uv(45.f, screen_size, coord);
	f32x3 ray_origin = f32x3(0.f, 10.f, 0.f);
	ray_origin.x += cosG(bc->time_) * 100.f;
	ray_origin.y += sinG(bc->time_) * 50.f;

	VoxelModel model = bc->models[0];
	f32x4 color = f32x4s(0.f);
	f32x3 aabb_center = model.position;
	f32x3 aabb_min = subG(aabb_center, model.half_size);
	f32x3 aabb_max = addG(aabb_center, model.half_size);

	f32x3 ray_origin_local_space = subG(ray_origin, model.position);
	f32x2 xz = f32x2(ray_origin_local_space.x, ray_origin_local_space.z);
	xz = vmulG(xz, voxel_mat2_identity_rotation(bc->time_));
	ray_origin_local_space.x = xz.x;
	ray_origin_local_space.z = xz.y;
	ray_origin_local_space = addG(ray_origin_local_space, model.position);

	f32x3 ray_dir_local_space = ray_dir;
	xz = f32x2(ray_dir_local_space.x, ray_dir_local_space.z);
	xz = vmulG(xz, voxel_mat2_identity_rotation(bc->time_));
	ray_dir_local_space.x = xz.x;
	ray_dir_local_space.z = xz.y;

	AabbVsRayHit hit = aabb_vs_ray3d(aabb_min, aabb_max, ray_origin_local_space, ray_dir_local_space);
	if (hit.success) {
		float t = hit.tmin;
		while (t < hit.tmax) {
			f32x3 pt_world_space = addG(ray_origin_local_space, mulsG(ray_dir_local_space, t));
			f32x3 pt_local_space = subG(pt_world_space, aabb_min);
			pt_local_space = floorG(pt_local_space);
			f32x4 texel = load_textureG(model.color, u32x3(pt_local_space.x, pt_local_space.y, pt_local_space.z));
			if (texel.a >= VOXEL_EPSILON) {
				color = texel;
				break;
			}
			t += 1.f;
		}
	}

	uint32_t int32_cast_test = (uint32_t)bc->output;
	HccRwTexture2D(f32x4) resource_cast_test = (HccRwTexture2D(f32x4))int32_cast_test;
	store_textureG(resource_cast_test, u32x2(sv->dispatch_idx.x, sv->dispatch_idx.y), color);
}

#endif // __HCC__
