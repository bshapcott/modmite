#if 0

// - where old code goes to die

//____________________________________________________________________________
// Base Address Properties->Linker->Advanced->Base Address
// @..\..\..\os\win32\BaseAddr.ref,mod_helloworld.so

//____________________________________________________________________________
static void do_javascript(request_rec *r) {
	JSContext *cx;
	JSObject *global_obj;
	JSClass global_class = {
		"global", 0, JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
			JS_PropertyStub, JS_EnumerateStub, JS_ResolveStub,
			JS_ConvertStub, JS_FinalizeStub
	};
	my_server_config *config;
	char *script;
	jsval rval;
	JSString *str;
	JSBool ok;

	config = ap_get_module_config(r->server->module_config,
		&helloworld_module);

	if (!config) goto js_error;

	ap_rputs("<H1>Spidermonkey JavaScript 1.6</H1>\n", r);

	if (NULL == (cx = JS_NewContext(config->rt, 0x1000))) goto js_error;

	global_obj = JS_NewObject(cx, &global_class, NULL, NULL);
	JS_InitStandardClasses(cx, global_obj);

	script = "22/7";

	ok = JS_EvaluateScript(cx, global_obj, script, strlen(script),
		"internal", 0, &rval);
	str = JS_ValueToString(cx, rval);
	ap_rputs("script result ", r);
	ap_rputs(JS_GetStringBytes(str), r);

	ap_rputs("\n<HR/>\n", r);

js_error:
	return;
}

//____________________________________________________________________________
static int print_item(void* rec, const char* key, const char* value) {
	/* rec is a userdata pointer.  We'll pass the request_rec in it */
	request_rec *r = rec;
	ap_rprintf(r, "<tr><th scope=\"row\">%s</th><td>%s</td></tr>\n",
		ap_escape_html(r->pool, key), ap_escape_html(r->pool, value));
	/* Zero would stop iterating; any other return value continues */
	return 1;
}

//____________________________________________________________________________
static void print_table(request_rec *r, apr_table_t *t,
					   const char *caption, const char *keyhead,
					   const char *valhead)
{

	/* print a table header */
	ap_rprintf(r, "<table><caption>%s</caption><thead>"
		"<tr><th scope=\"col\">%s</th><th scope=\"col\">%s"
		"</th></tr></thead><tbody>", caption, keyhead, valhead);

	/* Print the data: apr_table_do iterates over entries with our callback */
	apr_table_do(print_item, r, t, NULL);

	/* and finish the table */
	ap_rputs("</tbody></table>\n", r);
	return;
}

#define REQUEST_ROW(x) 	ap_rprintf(r, "<tr><td><b>" #x "</b></td><td>%s</td></tr>\n", r->x)

//____________________________________________________________________________
static void print_request(request_rec *r) {
	ap_rputs("<hr/>\n", r);
	ap_rputs("<table>\n", r);
	REQUEST_ROW(unparsed_uri);
	REQUEST_ROW(uri);
	REQUEST_ROW(filename);
	REQUEST_ROW(canonical_filename);
	REQUEST_ROW(path_info);
	REQUEST_ROW(args);
	ap_rputs("</table>\n", r);
	return;
}

#undef REQUEST_ROW

//____________________________________________________________________________
// Simple sanity test of the JS runtime lifecycle.
static void js_test(my_server_config *config) {
	// JSRuntime *rt;
	JSContext *cx;
	JSObject  *glob;
	JSBool builtins;
	jsval rval;
	JSString *str;
	JSBool ok;
	char *script = "4*27";

	config->rt = JS_NewRuntime(8L * 1024L * 1024L);
	// note that at least one context must be created during the runtime's
	// lifecycle, or an exception will occur when the runtime is destroyed
	cx = JS_NewContext(config->rt, 8192);
	glob = JS_NewObject(cx, NULL, NULL, NULL);
	builtins = JS_InitStandardClasses(cx, glob);
	ok = JS_EvaluateScript(cx, glob, script, strlen(script),
		"internal", 0, &rval);
	str = JS_ValueToString(cx, rval);
	printf("script result: %s\n", JS_GetStringBytes(str));
	JS_DestroyContext(cx);
	JS_DestroyRuntime(config->rt);
	return;
}

#endif
