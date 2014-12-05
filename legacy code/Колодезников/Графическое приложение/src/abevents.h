/* Event header for application - AppBuilder 2.03  */

static const ApEventLink_t AbApplLinks[] = {
	{ 3, 0, 0L, 0L, 0L, &base, NULL, NULL, 0, NULL, 0, 0, 0, 0, },
	{ 0 }
	};

static const ApEventLink_t AbLinks_base[] = {
	{ 8, 3, 0L, 0L, 0L, NULL, NULL, "PtRawDots", 24000, (int(*)(PtWidget_t*,ApInfo_t*,PtCallbackInfo_t*)) my_raw_draw_fn, 0, 0, 0, 0, },
	{ 0 }
	};

const char ApOptions[] = AB_OPTIONS;
