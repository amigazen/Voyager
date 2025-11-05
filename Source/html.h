/**************************************************************************

  =======================
  The Voyager Web Browser
  =======================

  Copyright (C) 1995-2001 by
   Oliver Wagner <owagner@vapor.com>
   All Rights Reserved

  Parts Copyright (C) by
   David Gerber <zapek@vapor.com>
   Jon Bright <jon@siliconcircus.com>
   Matt Sealey <neko@vapor.com>

**************************************************************************/


#ifndef VOYAGER_HTML_H
#define VOYAGER_HTML_H
/*
 *$Id: html.h,v 1.16 2001/07/01 22:02:44 owagner Exp $
 */

//
//  HTML token definitions
//

enum {
	ht_dummy = 256,
	ht_h1,
	ht_h2,
	ht_h3,
	ht_h4,
	ht_h5,
	ht_h6,
	ht_b,
	ht_i,
	ht_tt,
	ht_blockquote,
	ht_strong,
	ht_s,
	ht_center,
	ht_hr,
	ht_a,
	ht_title,
	ht_br,
	ht_pre,
	ht_ul,
	ht_li,
	ht_p,
	ht_menu,
	ht_dir,
	ht_ol,
	ht_img,
	ht_dl,
	ht_dt,
	ht_dd,
	ht_form,
	ht_input,
	ht_button,
	ht_select,
	ht_option,
	ht_table,
	ht_tr,
	ht_td,
	ht_th,
	ht_textarea,
	ht_em,
	ht_address,
	ht_body,
	ht_tabstop,
	ht_isindex,
	ht_nobr,
	ht_map,
	ht_area,
	ht_base,
	ht_xmp,
	ht_listing,
	ht_frame,
	ht_caption,
	ht_colgroup,
	ht_thead,
	ht_tbody,
	ht_font,
	ht_script,
	ht_frameset,
	ht_noframes,
	ht_div,
	ht_span,
	ht_big,
	ht_small,
	ht_meta,
	ht_u,
	ht_applet,
	ht_param,
	ht_embed,
	ht_nbsp,
	ht_style,
	ht_noscript,
	ht_noembed,
	ht_comment,
	ht_basefont,
	ht_strike,
	ht_del,
	ht_ins,
	ht_code,
	ht_samp,
	ht_kbd,
	ht_cite,
	ht_dfn,
	ht_var
};

#define HTF_NEGATE 1024

#endif /* VOYAGER_HTML_H */
