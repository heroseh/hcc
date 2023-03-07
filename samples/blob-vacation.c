
#include <stdbool.h>
#include <stdint.h>
#include <hmaths/types.h>
#include <hcc_shader.h>

typedef struct BlobVacationBC BlobVacationBC;
struct BlobVacationBC {
	float time_;
	uint32_t screen_width;
	uint32_t screen_height;
};

#ifdef __HCC__
#include <hmaths/maths.h>

HCC_INVOCATION const float BLOB_MIN_DIST = 0.f;
HCC_INVOCATION const float BLOB_MAX_DIST = 100.f;
HCC_INVOCATION const float BLOB_EPSILON = 0.0001f;
HCC_INVOCATION const int BLOB_MAX_MARCHING_STEPS = 1024;

typedef struct BlobVacationFragment BlobVacationFragment;
HCC_FRAGMENT_STATE struct BlobVacationFragment {
	f32x4 color;
};

HCC_VERTEX void blob_vacation_vs(HccVertexSV const* const sv, HccVertexSVOut* const sv_out, BlobVacationBC const* const bc, void* const state_out) {
	sv_out->position = f32x4((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f, 0.f, 1.f);
}

//
// these random and noise functions are from this document:
// https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
//

float rand_f32(float n){return fractG(sinG(n) * 43758.5453123f);}

float noise_f32(float p){
	float fl = floorG(p);
	float fc = fractG(p);
	return lerpG(rand_f32(fl), rand_f32(fl + 1.f), fc);
}

float rand_f32x2(f32x2 n) {
	return fractG(sinG(dotG(n, f32x2(12.9898f, 4.1414f))) * 43758.5453f);
}

float noise_f32x2(f32x2 p){
	f32x2 ip = floorG(p);
	f32x2 u = fractG(p);
	u = mulG(mulG(u, u), ssubG(3.f, mulsG(u, 2.f)));

	float res = lerpG(
		lerpG(rand_f32x2(ip),rand_f32x2(addG(ip, f32x2(1.f,0.f))),u.x),
		lerpG(rand_f32x2(addG(ip, f32x2(0.f,1.f))),rand_f32x2(addG(ip, f32x2(1.f,1.f))),u.x),u.y);
	return res*res;
}

//
// the sd_* are signed distance functions working out the
// distance from the ray marching sample point. the distance
// is packed into the x component and the color is stored in yzw.
//
// there is a great catalog of signed distanced functions here:
// https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//


f32x4 sd_sphere(f32x3 sample_pt, f32x3 center, float radius, f32x3 color) {
	float dist = lenG(subG(sample_pt, center)) - radius;
	return f32x4(dist, color.x, color.y, color.z);
}

f32x4 sd_plane(f32x3 sample_pt, float y, f32x3 color) {
	float dist = sample_pt.y - y;
	return f32x4(dist, color.x, color.y, color.z);
}

f32x4 sd_terrain(f32x3 sample_pt, f32x3 color) {
	float dist = sample_pt.y - noise_f32x2(divsG(f32x2(sample_pt.x, sample_pt.z), 5.f))*5.f+1.f;
	color.g += sample_pt.y * 0.35f; // add green to the tips of the terrain
	return f32x4(dist, color.x, color.y, color.z);
}

// chooses the closest shape
f32x4 sd_shape_min(f32x4 shape_a, f32x4 shape_b) {
	if (shape_a.x < shape_b.x) return shape_a;
	return shape_b;
}

float distort(f32x3 p) {
	return sinG(2.f*p.x)*sinG(2.f*p.y)*sinG(2.f*p.z);
}

f32x4x4 gen_mat_rotate_y(float theta) {
	float c = cosG(theta);
	float s = sinG(theta);

	f32x4x4 m = {0};
	m.cols[0].x = c;
	m.cols[0].z = s;
	m.cols[1].y = 1.f;
	m.cols[2].x = -s;
	m.cols[2].z = c;
	m.cols[3].w = 1.f;

	return m;
}

f32x4 map_world(f32x3 sample_pt, float time_) {
	f32x3 sphere_pos = f32x3(4.f, sinG(time_) + 2.5f, 5.5f);

	// rotate the sphere around world origin 0, 0, 0
	f32x4 v = mulvG(gen_mat_rotate_y(time_), f32x4(sphere_pos.x, sphere_pos.y,  sphere_pos.z, 1.f));
	sphere_pos = f32x3(v.x, v.y, v.z);

	f32x4 sphere = sd_sphere(sample_pt, sphere_pos, 2.f, f32x3(0.8f, 0.2f, 0.8f));
	sphere.x += distort(addsG(mulsG(sample_pt, sinG(time_ * 0.05f)), 1.f));
	sphere.x *= 0.5f;

	f32x4 terrain = sd_terrain(addsG(sample_pt, 2.f), f32x3(0.3f, 0.3f, 0.6f));
	terrain.x *= 0.7f;

	return sd_shape_min(terrain, sphere);
}

//
// marches a ray starting from 'start' until we find a shape close enough
// or we have gone past the 'end'
f32x4 ray_march_get_closest_shape(f32x3 ray_orgin, f32x3 march_dir, float start, float end, float time_) {
	float depth = start;
	for (int i = 0; i < BLOB_MAX_MARCHING_STEPS; i++) {
		f32x4 shape = map_world(addG(ray_orgin, mulsG(march_dir, depth)), time_);
		float dist = shape.x;
		if (dist < BLOB_EPSILON) {
			shape.x = depth;
			return shape;
		}
		depth += dist;
		if (depth >= end) {
			return f32x4(end, 0.f, 0.f, 0.f);
		}
	}
	return f32x4(end, 0.f, 0.f, 0.f);
}

// calculate the view space ray direction (that we use for marching) from a uv coordinate
f32x3 ray_direction_for_uv(float fov, f32x2 size, f32x2 uv) {
	f32x2 xy = subG(uv, divsG(size, 2.f));
	float z = size.y / tanG(radiansG(fov) / 2.f);
	return normG(f32x3(xy.x, -xy.y, -z));
}

// map_world(...).x returns <= 0.f when the sample point is inside the closest shape and > 0.f when outside.
// we use this to shift each component by a little each way and subtract to roughly see how much
// that component contributes to the surface normal.
f32x3 estimate_normal(f32x3 p, float time_) {
	return normG(f32x3(
		map_world(f32x3(p.x + BLOB_EPSILON, p.y, p.z), time_).x - map_world(f32x3(p.x - BLOB_EPSILON, p.y, p.z), time_).x,
		map_world(f32x3(p.x, p.y + BLOB_EPSILON, p.z), time_).x - map_world(f32x3(p.x, p.y - BLOB_EPSILON, p.z), time_).x,
		map_world(f32x3(p.x, p.y, p.z  + BLOB_EPSILON), time_).x - map_world(f32x3(p.x, p.y, p.z - BLOB_EPSILON), time_).x
	));
}

//
// the diffuse contribution from a light source
f32x3 diffuse_contribution(f32x3 diffuse_color, f32x3 p, f32x3 camera_pos, f32x3 light_dir, f32x3 light_intensity, float time_) {
	f32x3 N = estimate_normal(p, time_);
	f32x3 L = normG(light_dir);

	float NdotL = dotG(N, L);
	return mulG(light_intensity, mulsG(diffuse_color, NdotL));
}

f32x3 phong_illumination(f32x3 ambient_color, f32x3 diffuse_color, f32x3 p, f32x3 camera_pos, float time_) {
	const f32x3 ambient_light = f32x3(0.7f, 0.7f, 0.9f);
	f32x3 color = mulG(ambient_light, ambient_color);

	//
	// fake the sun's light
	f32x3 light_pos = f32x3(0.f, 18.f, 50.5f);
	f32x3 light_intensity = f32x3s(3.5f);
	color = addG(color, diffuse_contribution(diffuse_color, p, camera_pos, light_pos, light_intensity, time_));
	return color;
}

f32x4x4 gen_mat_view(f32x3 camera_pos, f32x3 center, f32x3 up) {
	f32x3 f = normG(subG(center, camera_pos));
	f32x3 s = normG(crossG(f, up));
	f32x3 u = crossG(s, f);

	f = negG(f);

	f32x4x4 m = {0};
	m.cols[0].x = s.x;
	m.cols[0].y = s.y;
	m.cols[0].z = s.z;

	m.cols[1].x = u.x;
	m.cols[1].y = u.y;
	m.cols[1].z = u.z;

	m.cols[2].x = f.x;
	m.cols[2].y = f.y;
	m.cols[2].z = f.z;

	m.cols[3].w = 1.f;
	return m;
}

HCC_FRAGMENT void blob_vacation_fs(HccFragmentSV const* const sv, HccFragmentSVOut* const sv_out, BlobVacationBC const* const bc, void const* const state, BlobVacationFragment* const frag_out) {
	f32x2 screen_size = f32x2(bc->screen_width, bc->screen_height);

	f32x3 view_dir = ray_direction_for_uv(45.f, screen_size, f32x2(sv->frag_coord.x, sv->frag_coord.y));
	f32x3 camera_pos = f32x3(4.f, 6.f, 42.f);

	// animate camera
	camera_pos.x += sinG(bc->time_ * 0.6f) * 9.f;
	camera_pos.y += sinG(bc->time_ * 0.7f) * 3.f;

	// translate our ray direction from view space to world space
	f32x4x4 view_to_world = gen_mat_view(camera_pos, f32x3(0.f, 0.f, 0.f), f32x3(0.f, 1.f, 0.f));
	f32x4 temp_world_dir = mulvG(view_to_world, f32x4(view_dir.x, view_dir.y, view_dir.z, 0.f));
	f32x3 world_dir = f32x3(temp_world_dir.x, temp_world_dir.y, temp_world_dir.z);

	// ray march to get the closest distance and color for the shape we hit
	f32x4 shape = ray_march_get_closest_shape(camera_pos, world_dir, BLOB_MIN_DIST, BLOB_MAX_DIST, bc->time_);
	float dist = shape.x;
	f32x3 color = f32x3(shape.y, shape.z, shape.w);

	if (dist > BLOB_MAX_DIST - BLOB_EPSILON) {
		// did not hit anything so render the sky

		f32x3 sun_dir = normG(f32x3(0.f, 0.09f, -1.f));
		float sun = clampG(dotG(sun_dir, world_dir), 0.f, 1.f);
		f32x3 col = subsG(f32x3(0.6f, 0.71f, 0.75f), world_dir.y * 0.1f + 0.01f);

		// sun sky glow
		col = addG(col, mulsG(f32x3(1.f, 0.6f, 0.1f), 0.2f * powG(sun, 32.f)));

		// sun
		col = addG(col, mulsG(f32x3(1.f, 0.6f, 0.1f), smoothstepG(0.997f, 1.f, sun) * 1.2f));

		// sun glare
		col = addG(col, mulsG(f32x3(1.f, 0.4f, 0.2f), 0.2f * powG(sun, 32.f)));

		frag_out->color = f32x4(col.x, col.y, col.z, 1.f);
		return;
	}

	// the closest ray marched point we hit to the camera for this fragment
	f32x3 p = addG(camera_pos, mulsG(world_dir, dist));

	f32x3 ambient_color = mulG(f32x3(0.2f, 0.2f, 0.2f), color);
	f32x3 diffuse_color = mulG(f32x3(0.7f, 0.2f, 0.2f), color);

	color = phong_illumination(ambient_color, diffuse_color, p, camera_pos, bc->time_);

	frag_out->color = f32x4(color.x, color.y, color.z, 1.f);
}

#endif // __HCC__

