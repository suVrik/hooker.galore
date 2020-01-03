$input v_texcoord0

#include <bgfx_shader.sh>
#include <shader_utils.sh>

SAMPLER2D(s_texture, 0);

#define EDGE_THRESHOLD_MIN 0.0312
#define EDGE_THRESHOLD_MAX 0.125
#define QUALITY(q) ((q) < 5 ? 1.0 : ((q) > 5 ? ((q) < 10 ? 2.0 : ((q) < 11 ? 4.0 : 8.0)) : 1.5))
#define ITERATIONS 12
#define SUBPIXEL_QUALITY 0.75

float rgb2luma(vec3 rgb) {
    return sqrt(dot(rgb, vec3(0.299, 0.587, 0.114)));
}

/* FXAA as described in the Nvidia FXAA 3.11 white paper. */
void main() {
    vec2 uv = to_uv(v_texcoord0);
    vec3 color = texture2D(s_texture, uv).rgb;

    float luma       = rgb2luma(color);
    float luma_down  = rgb2luma(texture2D(s_texture, uv + vec2(0.0, -u_viewTexel.y)).rgb);
    float luma_up    = rgb2luma(texture2D(s_texture, uv + vec2(0.0,  u_viewTexel.y)).rgb);
    float luma_left  = rgb2luma(texture2D(s_texture, uv + vec2(-u_viewTexel.x, 0.0)).rgb);
    float luma_right = rgb2luma(texture2D(s_texture, uv + vec2( u_viewTexel.x, 0.0)).rgb);
    float luma_min   = min(luma, min(min(luma_down, luma_up), min(luma_left, luma_right)));
    float luma_max   = max(luma, max(max(luma_down, luma_up), max(luma_left, luma_right)));
    float luma_range = luma_max - luma_min;

    // If the luma variation is lower that a threshold, don't perform any AA.
    if (luma_range < max(EDGE_THRESHOLD_MIN, luma_max * EDGE_THRESHOLD_MAX)) {
        gl_FragColor = vec4(color, 1.0);
        return;
    }

    // Query the 4 remaining corners lumas.
    float luma_down_left  = rgb2luma(texture2D(s_texture, uv + vec2(-u_viewTexel.x, -u_viewTexel.y)).rgb);
    float luma_up_right   = rgb2luma(texture2D(s_texture, uv + vec2( u_viewTexel.x,  u_viewTexel.y)).rgb);
    float luma_up_left    = rgb2luma(texture2D(s_texture, uv + vec2(-u_viewTexel.x,  u_viewTexel.y)).rgb);
    float luma_down_right = rgb2luma(texture2D(s_texture, uv + vec2( u_viewTexel.x, -u_viewTexel.y)).rgb);
    // Combine the edges and corners lumas.
    float luma_down_up       = luma_down + luma_up;
    float luma_left_right    = luma_left + luma_right;
    float luma_left_corners  = luma_down_left + luma_up_left;
    float luma_down_corners  = luma_down_left + luma_down_right;
    float luma_right_corners = luma_down_right + luma_up_right;
    float luma_up_corners    = luma_up_right + luma_up_left;

    // Compute an estimation of the gradient along the horizontal and vertical axis.
    float edge_horizontal = abs(-2.0 * luma_left + luma_left_corners) + abs(-2.0 * luma + luma_down_up ) * 2.0   + abs(-2.0 * luma_right + luma_right_corners);
    float edge_vertical   = abs(-2.0 * luma_up + luma_up_corners)     + abs(-2.0 * luma + luma_left_right) * 2.0 + abs(-2.0 * luma_down + luma_down_corners);

    bool is_horizontal = edge_horizontal >= edge_vertical;

    // Choose the step size (one pixel) accordingly.
    float step_size = is_horizontal ? u_viewTexel.y : u_viewTexel.x;

    // Select the two neighboring texels lumas in the opposite direction to the local edge.
    float luma1 = is_horizontal ? luma_down : luma_left;
    float luma2 = is_horizontal ? luma_up : luma_right;
    // Compute gradients in this direction.
    float gradient1 = luma1 - luma;
    float gradient2 = luma2 - luma;

    // Gradient in the corresponding direction, normalized.
    float gradient_scaled = 0.25 * max(abs(gradient1), abs(gradient2));

    // Average luma in the correct direction.
    float luma_average = 0.0;
    if (abs(gradient1) >= abs(gradient2)) {
        // Switch the direction
        step_size = -step_size;
        luma_average = 0.5 * (luma1 + luma);
    } else {
        luma_average = 0.5 * (luma2 + luma);
    }

    // Shift UV to the correct direction by half a pixel.
    vec2 uv_tmp = uv;
    if (is_horizontal) {
        uv_tmp.y += step_size * 0.5;
    } else {
        uv_tmp.x += step_size * 0.5;
    }

    vec2 offset = is_horizontal ? vec2(u_viewTexel.x, 0.0) : vec2(0.0, u_viewTexel.y);
    // Compute UVs to explore on each side of the edge, orthogonally.
    vec2 uv1 = uv_tmp - offset * QUALITY(0);
    vec2 uv2 = uv_tmp + offset * QUALITY(0);

    // Read the lumas at both current extremities of the exploration segment, and compute the delta wrt to the local average luma.
    float luma_end1 = rgb2luma(texture2D(s_texture, uv1).rgb);
    float luma_end2 = rgb2luma(texture2D(s_texture, uv2).rgb);
    luma_end1 -= luma_average;
    luma_end2 -= luma_average;

    // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
    bool reached1 = abs(luma_end1) >= gradient_scaled;
    bool reached2 = abs(luma_end2) >= gradient_scaled;
    bool reached_both = reached1 && reached2;

    // If the side is not reached, we continue to explore in this direction.
    if (!reached1) {
        uv1 -= offset * QUALITY(1);
    }
    if (!reached2) {
        uv2 += offset * QUALITY(1);
    }

    // If both sides have not been reached, continue to explore.
    if (!reached_both) {
        for (int i = 2; i < ITERATIONS; i++) {
            if (!reached1) {
                luma_end1 = rgb2luma(texture2D(s_texture, uv1).rgb);
                luma_end1 = luma_end1 - luma_average;
            }
            if (!reached2) {
                luma_end2 = rgb2luma(texture2D(s_texture, uv2).rgb);
                luma_end2 = luma_end2 - luma_average;
            }
            // If the luma deltas at the current extremities is larger than the local gradient, we have reached the side of the edge.
            reached1 = abs(luma_end1) >= gradient_scaled;
            reached2 = abs(luma_end2) >= gradient_scaled;
            reached_both = reached1 && reached2;

            // If the side is not reached, continue to explore in this direction, with a variable quality.
            if (!reached1) {
                uv1 -= offset * QUALITY(i);
            }
            if (!reached2) {
                uv2 += offset * QUALITY(i);
            }

            // If both sides have been reached, stop the exploration.
            if (reached_both) {
                break;
            }
        }
    }

    // Compute the distances to each side edge of the edge (!).
    float distance1 = is_horizontal ? (uv.x - uv1.x) : (uv.y - uv1.y);
    float distance2 = is_horizontal ? (uv2.x - uv.x) : (uv2.y - uv.y);

    // Thickness of the edge.
    float edge_thickness = (distance1 + distance2);

    // If the luma at center is smaller than at its neighbour, the delta luma at each end should be positive (same variation).
    bool correct_variation1 = (luma_end1 < 0.0) != (luma < luma_average);
    bool correct_variation2 = (luma_end2 < 0.0) != (luma < luma_average);

    // Only keep the result in the direction of the closer side of the edge.
    bool correct_variation = distance1 < distance2 ? correct_variation1 : correct_variation2;

    // UV offset: read in the direction of the closest side of the edge.
    float pixel_offset = -min(distance1, distance2) / edge_thickness + 0.5;

    // If the luma variation is incorrect, do not offset.
    float final_offset = correct_variation ? pixel_offset : 0.0;

    // Sub-pixel shifting
    // Full weighted average of the luma over the 3x3 neighborhood.
    luma_average = (1.0 / 12.0) * (2.0 * (luma_down_up + luma_left_right) + luma_left_corners + luma_right_corners);
    // Ratio of the delta between the global average and the center luma, over the luma range in the 3x3 neighborhood.
    float subpixel_offset1 = clamp(abs(luma_average - luma) / luma_range, 0.0, 1.0);
    float subpixel_offset2 = (-2.0 * subpixel_offset1 + 3.0) * subpixel_offset1 * subpixel_offset1;
    // Compute a sub-pixel offset based on this delta.
    float subpixel_offset = subpixel_offset2 * subpixel_offset2 * SUBPIXEL_QUALITY;

    // Pick the biggest of the two offsets.
    final_offset = max(final_offset, subpixel_offset);

    // Compute the final UV coordinates.
    if (is_horizontal) {
        uv.y += final_offset * step_size;
    } else {
        uv.x += final_offset * step_size;
    }

    vec3 color_out = texture2D(s_texture, uv).rgb;
    gl_FragColor = vec4(color_out, 1.0);
}
