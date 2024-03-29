
#include <stdbool.h>
#include <stdint.h>
#include <hmaths_types.h>
#include <hcc_shader.h>

typedef struct ColorPickerBC ColorPickerBC;
struct ColorPickerBC {
	float    time_;
	uint32_t screen_width;
	uint32_t screen_height;
};

#ifdef __HCC__
#include <hmaths.h>

#define MAX_ITER 1000
#define MAX_DIST 9.f
#define EPSILON 0.001f

#define CUBE_AXIS_COUNT	   4
#define CUBE_HALF_SIZE		(CUBE_SIZE / 2.f)
#define CUBE_BORDER_HALF_SIZE (0.1f / (float)(CUBE_AXIS_COUNT * CUBE_AXIS_COUNT + 1))
#define CUBE_SIZE			 ((2.f - CUBE_BORDER_HALF_SIZE) / (float)(CUBE_AXIS_COUNT * CUBE_AXIS_COUNT + 1))
#define CUBE_AXIS_OFFSET	  (CUBE_SIZE * (float)(CUBE_AXIS_COUNT) + CUBE_BORDER_HALF_SIZE)
#define SELECTED_SPEED		4.f
#define SELECTED_MAX_OFFSET   (CUBE_HALF_SIZE * 2.f)

//
// these sinG functions (mostly ANIM_WAVE) is slowing down the shader.
// in a real application these will be passed in to the shader.
#define ANIM_ROTATION_Y	   (is_at_timeline_range(TIMELINE_SPIN_START, TIMELINE_SPIN_END, time_) ? sinG(time_ * 1.5f) * PI_F32 * 2.f : 0.f)
#define ANIM_AXIS			 (is_at_timeline_range(TIMELINE_OPEN_START, TIMELINE_OPEN_END, bc->time_) ? (sinG(bc->time_ * 2.f - PI_F32 / 2.f) * 0.5f + 0.5f) : 1.f)
#define ANIM_WAVE			 (is_at_timeline_range(TIMELINE_WAVE_START, TIMELINE_WAVE_END, time_) ? sinG(time_ * 8.f + (float)(color_idx.x + color_idx.z)) * (sinG(smoothstepG(TIMELINE_WAVE_START, TIMELINE_WAVE_END, time_) * (PI_F32 * 1.5f)) * 0.5f + 0.5f) * 0.045f : 0.f)
#define ANIM_SELECTED		 (is_at_timeline_range(TIMELINE_SELECT_START, TIMELINE_SELECT_END, time_) ? 1.f : 0.f)

#define TIMELINE_SPIN_START   0.f
#define TIMELINE_SPIN_END	 1.1f
#define TIMELINE_OPEN_START   0.f
#define TIMELINE_OPEN_END	 1.5f
#define TIMELINE_WAVE_START   0.75f
#define TIMELINE_WAVE_END	 1.6f
#define TIMELINE_SELECT_START 1.3f
#define TIMELINE_SELECT_END   INFINITY_F32

static const float cube_half_size = CUBE_HALF_SIZE;
static const float cube_size = CUBE_SIZE;
static const float cube_border_half_size = CUBE_BORDER_HALF_SIZE;
static const float cube_axis_offset = CUBE_AXIS_OFFSET;
static const float selected_max_offset = SELECTED_MAX_OFFSET;

typedef struct ColorPickerPixel ColorPickerPixel;
HCC_PIXEL_STATE struct ColorPickerPixel {
	f32x4 color;
};

HCC_VERTEX void color_picker_vs(HccVertexSV const* const sv, HccVertexSVOut* const sv_out, ColorPickerBC const* const bc, void* const state_out) {
	sv_out->position = f32x4((sv->vertex_idx & 1) * 2.f - 1.f, (sv->vertex_idx / 2) * 2.f - 1.f, 0.f, 1.f);
}

float distance_sq_cube(f32x3 pos, float size) {
	return lensqG(maxsG(subsG(absG(pos), size), 0.f));
}
float distance_sq_cube_frame(f32x3 pos, float size, float width) {
	pos = subsG(absG(pos), size);
	f32x3 q = subsG(absG(addsG(pos, width)), width);

	return minG(
		minG(
			lensqG(maxsG(f32x3(q.x, q.y, pos.z), 0.f)),
			lensqG(maxsG(f32x3(q.x, pos.y, q.z), 0.f))
		),
		lensqG(maxsG(f32x3(pos.x, q.y, q.z), 0.f))
	);
}

f32x2x2 mat2_identity_rotation(float angle)
{
	float ca = cosG(angle);
	float sa = sinG(angle);
	f32x2x2 m;
	m.cols[0].x = ca;
	m.cols[0].y = -sa;
	m.cols[1].x = sa;
	m.cols[1].y = ca;
	return m;
}

bool is_at_timeline_range(float start, float end, float time_) {
	return start <= time_ && time_ <= end;
}

f32x4 distance_cubes(
	f32x3 ray_sample_pos,
	f32x3 is_split_axis_v3,
	f32x3 split_axis_offset,
	u32x3 selected_color_idx,
	float offset_ratio,
	float time_
) {
	static const float scale = 1.f;

	//
	// rotate the sample position towards local space of the cube
#if 1
	f32x2 yz = f32x2(ray_sample_pos.y, ray_sample_pos.z);
	yz = vmulG(yz, mat2_identity_rotation(PI_F32 / 5.f));
	ray_sample_pos.y = yz.x;
	ray_sample_pos.z = yz.y;

	f32x2 xz = f32x2(ray_sample_pos.x, ray_sample_pos.z);
	xz = vmulG(xz, mat2_identity_rotation(PI_F32 / 4.f + ANIM_ROTATION_Y));
	ray_sample_pos.x = xz.x;
	ray_sample_pos.z = xz.y;
#else
	// TODO swizzle versino
	ray_sample_pos.yz *= mat2_identity_rotation(PI_F32 / 5.f);
	ray_sample_pos.xz *= mat2_identity_rotation(PI_F32 / 4.f + ANIM_ROTATION_Y);
#endif

	//
	// if CUBE_AXIS_COUNT == 3 then start_pos = -1.f, end_pos = 2.f
	// if CUBE_AXIS_COUNT == 4 then start_pos = -1.5f, end_pos = 2.5f
	f32x3 start_pos = f32x3s(-((float)(CUBE_AXIS_COUNT) - 1.f) / 2.f);
	f32x3 end_pos = addsG(start_pos, (float)(CUBE_AXIS_COUNT));

	f32x3 pos;
	f32x4 last_dist_sq = f32x4(0.f, 0.f, 0.f, 99999.f);
	u32x3 color_idx = u32x3(0, 0, 0);

	//
	// loop over all cubes
	for (pos.x = start_pos.x; pos.x < end_pos.x; pos.x += 1.f, color_idx.x += 1) {

		pos.y = start_pos.y;
		color_idx.y = 0;
		for (; pos.y < end_pos.y; pos.y += 1.f, color_idx.y += 1) {

			pos.z = start_pos.z;
			color_idx.z = 0;
			for (; pos.z < end_pos.z; pos.z += 1.f, color_idx.z += 1) {

				//
				// 0.f or 1.f if this cube is selected
				float is_selected_f = (float)(allG(eqG(selected_color_idx, color_idx)));

				//
				// calculate the selected offset if the cube is selected
				f32x3 selected_offset = mulsG(is_split_axis_v3, is_selected_f * selected_max_offset * offset_ratio * ANIM_SELECTED);

				//
				// the offset of the cube on the split axis
				f32x3 cube_split_axis_offset = mulG(pos, split_axis_offset);

				//
				// apply the wave and slow the demo down :/
				cube_split_axis_offset.y += ANIM_WAVE;

				//
				// bring the sample position in to local space of this cube
				f32x3 local_sample_pos = addG(ray_sample_pos, addG(cube_split_axis_offset, selected_offset));

				float next_dist_sq = distance_sq_cube(local_sample_pos, cube_half_size) * scale;
				if (next_dist_sq < last_dist_sq.w) {
					f32x3 color = divsG(f32x3(color_idx.x, color_idx.y, color_idx.z), (float)(CUBE_AXIS_COUNT - 1));
					float frame_dist_sq = distance_sq_cube_frame(local_sample_pos, cube_half_size, cube_border_half_size) * scale;
					if (frame_dist_sq < next_dist_sq) {
						//
						// we hit the frame of the cube
						float grey = sumelmtsG(divsG(ssubG(1.f, color), 3.f));
						grey += is_selected_f * sinG(time_ * SELECTED_SPEED) * ANIM_SELECTED;
						last_dist_sq = f32x4(grey, grey, grey, next_dist_sq);
					 } else {
						//
						// we hit the cube
						last_dist_sq = f32x4(color.x, color.y, color.z, next_dist_sq);
					}
				}
			}
		}
	}

	last_dist_sq.w = sqrtG(last_dist_sq.w);
	return last_dist_sq;
}

HCC_PIXEL void color_picker_ps(HccPixelSV const* const sv, HccPixelSVOut* const sv_out, ColorPickerBC const* const bc, void const* const state, ColorPickerPixel* const pixel_out) {
	f32x2 screen_size = f32x2(bc->screen_width, bc->screen_height);

	// a value from -1.f to 1.f
	f32x2 screen_pos = f32x2(
		(sv->pixel_coord.x * 2.f - screen_size.x) / screen_size.y,
		(sv->pixel_coord.y * 2.f - screen_size.y) / -screen_size.y
	);

	f32x3 world_up = f32x3(0.f, -1.f, 0.f);

	f32x3 camera_pos = f32x3(0.f, 0.f, 1.f);
	f32x3 camera_target = f32x3(0.f, 0.f, 0.f);
	f32x3 camera_dir = normG(subG(camera_target, camera_pos));
	f32x3 camera_right = crossG(camera_dir, world_up);
	f32x3 camera_up = crossG(camera_right, camera_dir);

	f32x3 ray_sample_pos = camera_pos;
	ray_sample_pos = addG(ray_sample_pos, mulsG(camera_right, screen_pos.x));
	ray_sample_pos = addG(ray_sample_pos, mulsG(camera_up, screen_pos.y));
	f32x3 ray_dir = normG(camera_dir);

	float total_dist = 0.f;
	f32x4 dist = f32x4(0.f, 0.f, 0.f, EPSILON);

	u32x3 selected_color_idx = u32x3(2, 2, 1);

	f32x3 axis = mulsG(f32x3(0.f, 1.f, 0.f), ANIM_AXIS);
	f32x3 axis_offset = addsG(mulsG(axis, cube_axis_offset), cube_size);

	float offset_ratio = sinG(bc->time_ * SELECTED_SPEED) * 0.5f + 0.5f;

	for (int i = 0; i < MAX_ITER; i += 1) {
		if (dist.w < EPSILON || total_dist > MAX_DIST) {
			break;
		}

		dist = distance_cubes(
			ray_sample_pos,
			axis,
			axis_offset,
			selected_color_idx,
			offset_ratio,
			bc->time_
		);
		total_dist += dist.w;
		ray_sample_pos = addG(ray_sample_pos, mulsG(ray_dir, dist.w));
	}

	f32x3 color = f32x3s(0.f);
	if (dist.w < EPSILON) {
		color.x = dist.x;
		color.y = dist.y;
		color.z = dist.z;
	}

	pixel_out->color = f32x4(color.x, color.y, color.z, 1.f);
}

#endif // __HCC__
