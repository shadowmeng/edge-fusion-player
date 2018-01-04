#include <windows.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <assert.h>
#include <d3d9.h>
#include <inttypes.h>
#include <limits.h>
#include "config.h"
#include "options/options.h"
#include "options/m_option.h"
#include "mpv_talloc.h"
#include "vo.h"
#include "video/csputils.h"
#include "video/mp_image.h"
#include "video/img_format.h"
#include "common/msg.h"
#include "common/common.h"
#include "w32_common.h"
#include "sub/osd.h"

#include "video/out/multid3d/multid3d_api.h"

#include "config.h"
#if !HAVE_GPL
#error GPL only
#endif









#define IMGFMT_IS_Y(x) ((x) == IMGFMT_Y8 || (x) == IMGFMT_Y16)
#define IMGFMT_Y_DEPTH(x) ((x) == IMGFMT_Y8 ? 8 : 16)






typedef struct multid3d_priv {
    struct mp_log *log;
	int image_format;
};

struct fmt_entry {
    const unsigned int  mplayer_fmt;   /**< Given by MPlayer */
    const D3DFORMAT     fourcc;        /**< Required by D3D's test function */
};

/* Map table from reported MPlayer format to the required
   fourcc. This is needed to perform the format query. */

static const struct fmt_entry fmt_table[] = {
    // planar YUV
    {IMGFMT_420P,  MAKEFOURCC('Y','V','1','2')},
    {IMGFMT_420P,  MAKEFOURCC('I','4','2','0')},
    {IMGFMT_420P,  MAKEFOURCC('I','Y','U','V')},
    {IMGFMT_NV12,  MAKEFOURCC('N','V','1','2')},
    // packed YUV
    {IMGFMT_UYVY,  D3DFMT_UYVY},
    // packed RGB
    {IMGFMT_BGR32, D3DFMT_X8R8G8B8},
    {IMGFMT_RGB32, D3DFMT_X8B8G8R8},
    {IMGFMT_BGR24, D3DFMT_R8G8B8}, //untested
    {IMGFMT_RGB565, D3DFMT_R5G6B5},
    // grayscale (can be considered both packed and planar)
    {IMGFMT_Y8,    D3DFMT_L8},
    {IMGFMT_Y16,   D3DFMT_L16},
    {0},
};

int check_format(uint32_t movie_fmt)
{
	const struct fmt_entry *cur = &fmt_table[0];
	while (cur->mplayer_fmt) {
        if (cur->mplayer_fmt == movie_fmt) {
			return cur->fourcc;
		}
		cur++;
	}
	return 0;
}


static int preinit(struct vo *vo)
{
	struct multid3d_priv *priv = (struct multid3d_priv *)vo->priv;
	priv->log = vo->log;

	MP_ERR(priv, "call multid3d_initialize\n");
	multid3d_initialize();
	return 0;
}

static int control(struct vo *vo, uint32_t request, void *data)
{
	printf("multid3d call control\n");
	return 0;
}

static int check_shader_conversion(struct multid3d_priv *priv, uint32_t fmt)
{
    struct mp_imgfmt_desc desc = mp_imgfmt_get_desc(fmt);
    if ((desc.flags & MP_IMGFLAG_YUV_P) && (desc.flags & MP_IMGFLAG_NE)) {
        if (desc.num_planes > MP_MAX_PLANES)
            return 0;
        int component_bits = desc.plane_bits;
        if (component_bits < 8 || component_bits > 16)
            return 0;
        bool is_8bit = component_bits == 8;
        //if (!is_8bit && priv->opt_only_8bit)
        //    return 0;
        //int texfmt = is_8bit ? IMGFMT_Y8 : IMGFMT_Y16;
        if(!is_8bit)
			return 1;
    }
    return 0;
}

static int reconfig(struct vo *vo, struct mp_image_params *params)
{
	printf("multid3d call reconfig\n");
	D3DMATRIX d3d_colormatrix;
	struct multid3d_priv *priv = (struct multid3d_priv *)vo->priv;
	struct mp_csp_params csp = MP_CSP_PARAMS_DEFAULTS;
    mp_csp_set_image_params(&csp, params);

	priv->image_format = params->imgfmt;
	struct mp_imgfmt_desc desc = mp_imgfmt_get_desc(priv->image_format);
	csp.input_bits = desc.plane_bits;

	csp.texture_bits = (csp.input_bits + 7) & ~7;
	struct mp_cmat coeff;
    mp_get_csp_matrix(&csp, &coeff);
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++){
                d3d_colormatrix.m[row][col] = coeff.m[row][col];
				printf("d3d_colormatrix.m[%d][%d]=%f\n", row, col, d3d_colormatrix.m[row][col]);
			}
            d3d_colormatrix.m[row][3] = coeff.c[row];
        }
	
	multi3d_setcolormatrix(&d3d_colormatrix);
	return 0; /* Success */
}

static void flip_page(struct vo *vo)
{

	multi3d_flip_page();
}

static void uninit(struct vo *vo)
{

}

static bool get_video_buffer(struct multid3d_priv *priv, struct mp_image *out)
{
	printf("multid3d call get_video_buffer\n");
	return true;
}

static void draw_image(struct vo *vo, mp_image_t *mpi)
{
	for (int n = 0; n < mpi->num_planes; n++)
	{
		multi3d_copyimg(mp_image_plane_w(mpi, n), mp_image_plane_h(mpi, n), mpi->fmt.bpp[n], mpi->planes[n], n);
	}

	multi3d_draw();
	
done:
    talloc_free(mpi);
}

static mp_image_t *get_window_screenshot(struct multid3d_priv *priv)
{

	return NULL;
}

static int imgfmt_any_rnd_depth(int fmt)
{
    if (IMGFMT_IS_RGB(fmt))
        return IMGFMT_RGB_DEPTH(fmt);
    if (IMGFMT_IS_Y(fmt))
        return IMGFMT_Y_DEPTH(fmt);
    assert(false);
    return 0;
}

static int query_format(struct vo *vo, int movie_fmt)
{
	//int bits_per_pixel = imgfmt_any_rnd_depth(movie_fmt);
	//printf("bits_per_pixel = %d\n", bits_per_pixel);
	struct multid3d_priv *priv = (struct multid3d_priv *)vo->priv;
	int fmt = check_format(movie_fmt);
	int support = check_shader_conversion(priv, movie_fmt);
	if (!support){	
		return 0;
	}
	
    return 1;
}

static const struct m_option opts[] = {
    {0}
};

const struct vo_driver video_out_multidirect3d = {
    .description = "Multi Direct3D 10 Renderer",
    .name = "multidirect3d",
    .preinit = preinit,
    .query_format = query_format,
    .reconfig = reconfig,
    .control = control,
    .draw_image = draw_image,
    .flip_page = flip_page,
    .uninit = uninit,
    .priv_size = sizeof(struct multid3d_priv),
    .options = opts,
    .options_prefix = "vo-multidirect3d",
};
