/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2018 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_thunder.h"

#include "Zend/zend_exceptions.h"	// for zend_throw_exception
#include "Zend/zend_interfaces.h"	// for zend_call_method_with_*
#include "Zend/zend_smart_str.h"	// for smart_str
#include "ext/standard/url.h"	// for php_url_*
#include "ext/standard/php_var.h"	// for php_var_dump
#include "ext/standard/php_string.h"	// for php_trim
#include "ext/standard/php_filestat.h"	// for php_stat
#include "ext/session/php_session.h"	// for php_session_start
#include "ext/json/php_json.h"	// for php_json_encode
#include "main/SAPI.h"	// for sapi_header_op
/* If you declare any globals in php_myclass.h uncomment this:*/
ZEND_DECLARE_MODULE_GLOBALS(thunder)
ZEND_DECLARE_MODULE_GLOBALS(thunder_internal)


/* True global resources - no need for thread safety here */
static int le_thunder;
double GetMicrotime()
{
	struct timeval tp = {0};
	if (gettimeofday(&tp, NULL)) {
		return (double)time(0);
	}
	return (double)(tp.tv_sec + tp.tv_usec / 1000000.00);
}
int zend_execute_scripts_ext(char *filepath){

    zval retval;

    zend_file_handle zfd;
    zfd.type = ZEND_HANDLE_FILENAME;
    zfd.filename = filepath;
    zfd.free_filename = 0;
    zfd.opened_path = NULL;

    //zend_execute_scripts(int type, zval *retval, int file_count, ...);
    //FAILURE OR SUCCESS
    return  zend_execute_scripts(ZEND_INCLUDE TSRMLS_CC,&retval,1,&zfd); 
}
zval * GlobalsStrFind(uint type, char *name, size_t len)
{
	zval *carrier, *field;
	carrier = &PG(http_globals)[type];
	if (!name) {
		return carrier;
	}
	field = zend_hash_str_find(Z_ARRVAL_P(carrier), name, len);
	if (!field) {
		return NULL;
	}
	return field;
}
int call_user_class_method(zval *retval, zend_class_entry *obj_ce, 
                           zval *obj, zval func,  uint32_t params_count, zval params[]){ 


    HashTable *function_table; 

    if(obj) { 
                function_table = &Z_OBJCE_P(obj)->function_table;
        }else{
                function_table = (CG(function_table));
    }

    zend_fcall_info fci;  
    fci.size = sizeof(fci);  
    fci.object =  obj ? Z_OBJ_P(obj) : NULL;;
    fci.function_name = func;   
    fci.retval = retval;  
    fci.param_count = params_count;  
    fci.params = params;  
    fci.no_separation = 1;  
 
    //FAILURE OR SUCCESS
    return  zend_call_function(&fci, NULL TSRMLS_CC);         //函数调用结束。  

}
/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("thunder.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_thunder_globals, thunder_globals)
    STD_PHP_INI_ENTRY("thunder.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_thunder_globals, thunder_globals)
PHP_INI_END()
*/
/* }}} */

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_thunder_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_thunder_compiled)
{
	char *arg = NULL;
	size_t arg_len, len;
	zend_string *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	strg = strpprintf(0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "thunder", arg);

	RETURN_STR(strg);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_thunder_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_thunder_init_globals(zend_thunder_globals *thunder_globals)
{
	thunder_globals->global_value = 0;
	thunder_globals->global_string = NULL;
}
*/
/* }}} */
/* {{{ thunder_functions[]
 *
 * Every user visible function must have an entry in thunder_functions[].
 */
const zend_function_entry thunder_functions[] = {
	PHP_ME(thunder_bootstrap, __construct, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_CTOR)
	// PHP_ME(thunder_bootstrap, set, arginfo_thunder_bootstrap_learn, ZEND_ACC_PUBLIC)
	// PHP_ME(thunder_bootstrap, test, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(thunder_bootstrap, init, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(thunder_bootstrap, run, NULL, ZEND_ACC_PUBLIC)

	PHP_FE_END	/* Must be the last line in thunder_functions[] */
};
/* }}} */
/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(thunder)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	AG(stringWeb) = zend_new_interned_string(zend_string_init(ZEND_STRL("WEB"), 1));
    AG(stringEn) = zend_new_interned_string(zend_string_init(ZEND_STRL("en_US"), 1));
    AG(stringSlash) = zend_new_interned_string(zend_string_init(ZEND_STRL("/"), 1));
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "ExtensionClass", thunder_functions);

	thunder_bootstrap_ce = zend_register_internal_class(&ce);
	zend_declare_property_null(thunder_bootstrap_ce, ZEND_STRL("attr1"), ZEND_ACC_PUBLIC);
      //声明一个静态数据成员app_dir
      zend_declare_property_string(thunder_bootstrap_ce, "app_dir", strlen("app_dir"), "",ZEND_ACC_PUBLIC|ZEND_ACC_STATIC TSRMLS_DC);
	return SUCCESS;
}
PHP_METHOD(thunder_bootstrap, __construct)
{
	
}
PHP_METHOD(thunder_bootstrap, init)
{
      zval *config = NULL, *field, *conf, *pData, iniValue;
	zend_string *environ = NULL, *iniName, *tstr, *docRoot = NULL, *baseUri = NULL, *uri = NULL, *appRoot = NULL;
	int module_number = 0;
	size_t len;
	char *cwd = NULL;
	double now;
	thunder_bootstrap_t *instance;

	if (zend_parse_parameters_throw(ZEND_NUM_ARGS(), "|zS", &config, &environ) == FAILURE) {
		return;
	}

	if (Z_TYPE(THUNDER_G(bootstrap)) != IS_UNDEF) {
		php_error_docref(NULL, E_ERROR, "Only one Azalea bootstrap can be initialized at a request");
		RETURN_FALSE;
	}

	// ---------- START ----------
	// print copyright
	sapi_header_line ctr = {0};
	ctr.line = PHP_THUNDER_NS_COPYRIGHT_OUTPUT;
	ctr.line_len = sizeof(PHP_THUNDER_NS_COPYRIGHT_OUTPUT) - 1;
	sapi_header_op(SAPI_HEADER_REPLACE, &ctr);

	// set timer
	now = GetMicrotime();
	THUNDER_G(timer) = now;

	// set environ
	if (environ && ZSTR_LEN(environ)) {
		zend_string_release(THUNDER_G(environ));
		THUNDER_G(environ) = zend_string_copy(environ);
	}

	// create output buffer
	if (php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE) {
		php_error_docref(NULL, E_ERROR, "Failed to create output buffer");
		RETURN_FALSE;
	}

	
	// set docRoot / base_uri / uri
	if ((cwd = VCWD_GETCWD(NULL, PATH_MAX))) {
		len = strlen(cwd);
		// 确保 / 结尾
		docRoot = zend_string_init(cwd, len + 1, 0);
		ZSTR_VAL(docRoot)[len] = DEFAULT_SLASH;
		ZSTR_VAL(docRoot)[len + 1] = '\0';
	} else {
		docRoot = AG(stringSlash);
	}
	THUNDER_G(docRoot) = docRoot;

	// load SERVER global variable
	if (PG(auto_globals_jit)) {
		tstr = zend_string_init(ZEND_STRL("_SERVER"), 0);
		zend_is_auto_global(tstr);
		zend_string_release(tstr);
	}

	if ((field = GlobalsStrFind(TRACK_VARS_SERVER, ZEND_STRL("SCRIPT_NAME"))) &&
			Z_TYPE_P(field) == IS_STRING) {
		baseUri = zend_string_dup(Z_STR_P(field), 0);
		// dirname
		len = zend_dirname(ZSTR_VAL(baseUri), ZSTR_LEN(baseUri));
		tstr = baseUri;
		if (len > 1) {
			// 确保 / 结尾
			baseUri = zend_string_alloc(len + 1, 0);
			memcpy(ZSTR_VAL(baseUri), ZSTR_VAL(tstr), len);
			ZSTR_VAL(baseUri)[len] = DEFAULT_SLASH;
			ZSTR_VAL(baseUri)[len + 1] = '\0';
		} else {
			// empty
			baseUri = AG(stringSlash);
		}
		zend_string_release(tstr);
	} else {
		// empty
		baseUri = AG(stringSlash);
	}
	do {
		if ((field = GlobalsStrFind(TRACK_VARS_SERVER, ZEND_STRL("PATH_INFO"))) &&
				Z_TYPE_P(field) == IS_STRING) {
			uri = zend_string_copy(Z_STR_P(field));
			break;
		}
		if ((field = GlobalsStrFind(TRACK_VARS_SERVER, ZEND_STRL("REQUEST_URI"))) &&
				Z_TYPE_P(field) == IS_STRING) {
			if (strncasecmp(Z_STRVAL_P(field), ZEND_STRL("http://")) &&
					strncasecmp(Z_STRVAL_P(field), ZEND_STRL("https://"))) {
				// not http url
				char *pos = strstr(Z_STRVAL_P(field), "?");
				if (pos) {
					// found query
					uri = zend_string_init(Z_STRVAL_P(field), pos - Z_STRVAL_P(field), 0);
				} else {
					uri = zend_string_copy(Z_STR_P(field));
				}
			} else {
				php_url *urlInfo = php_url_parse(Z_STRVAL_P(field));
				if (urlInfo && urlInfo->path) {
					uri = zend_string_init(urlInfo->path, strlen(urlInfo->path), 0);
				}
				php_url_free(urlInfo);
			}
			// remove baseUri
			if (0 == strncasecmp(ZSTR_VAL(uri), ZSTR_VAL(baseUri), ZSTR_LEN(baseUri))) {
				tstr = uri;
				uri = zend_string_init(ZSTR_VAL(uri) + ZSTR_LEN(baseUri), ZSTR_LEN(uri) - ZSTR_LEN(baseUri), 0);
				zend_string_release(tstr);
			}
			break;
		}
		if ((field = GlobalsStrFind(TRACK_VARS_SERVER, ZEND_STRL("ORIG_PATH_INFO"))) &&
				Z_TYPE_P(field) == IS_STRING) {
			uri = zend_string_copy(Z_STR_P(field));
			// remove baseUri
			if (0 == strncasecmp(ZSTR_VAL(uri), ZSTR_VAL(baseUri), ZSTR_LEN(baseUri))) {
				tstr = uri;
				uri = zend_string_init(ZSTR_VAL(uri) + ZSTR_LEN(baseUri), ZSTR_LEN(uri) - ZSTR_LEN(baseUri), 0);
				zend_string_release(tstr);
			}
			break;
		}
		// for CLI mode
		if ((field = GlobalsStrFind(TRACK_VARS_SERVER, ZEND_STRL("argv"))) &&
				Z_TYPE_P(field) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(field)) >= 2) {
			field = zend_hash_index_find(Z_ARRVAL_P(field), 1);
			uri = zend_string_copy(Z_STR_P(field));
			break;
		}
	} while (0);

	if (!baseUri) {
		baseUri = AG(stringSlash);
	}
	THUNDER_G(baseUri) = baseUri;

	if (uri) {
		zend_string *t = uri;
		uri = php_trim(uri, ZEND_STRL("/"), 3);
		zend_string_release(t);
	} else {
		uri = ZSTR_EMPTY_ALLOC();
	}
	THUNDER_G(uri) = uri;

	// define AZALEA magic const
	module_number = AG(moduleNumber);
	REGISTER_NS_STRINGL_CONSTANT(THUNDER_NS, "DOCROOT", ZSTR_VAL(docRoot), ZSTR_LEN(docRoot), CONST_CS);

	zend_string *ztemp ;
	len = strlen(cwd);
	ztemp = zend_string_init(cwd, len+1, 0);
	ZSTR_VAL(ztemp)[len] = DEFAULT_SLASH;
	ZSTR_VAL(ztemp)[len + 1] = '\0';
	char *temp = ZSTR_VAL(ztemp);
	strcat(temp,"app");
	len = strlen(temp);
	appRoot = zend_string_init(temp, len + 1, 0);

	ZSTR_VAL(appRoot)[len] = DEFAULT_SLASH;
	ZSTR_VAL(appRoot)[len + 1] = '\0';
	THUNDER_G(appRoot) = appRoot;

}
PHP_METHOD(thunder_bootstrap, run){
	
	zend_string *uri;
	zval *conf, *field, *paths;
	uri = THUNDER_G(uri);
	zend_string *stringSlash;
	char *temp = ZSTR_VAL(uri);
//   php_printf("%s\n",temp);

	zend_ulong pathsOffset = 0;
	paths = &THUNDER_G(paths);
	if (ZSTR_LEN(uri)) {
		php_explode( zend_new_interned_string(zend_string_init(ZEND_STRL("/"), 1)), uri, paths, ZEND_LONG_MAX);
	}
	field = zend_hash_index_find(Z_ARRVAL_P(paths), pathsOffset);
	THUNDER_G(controllerName) = zend_string_tolower(Z_STR_P(field));


	field = zend_hash_index_find(Z_ARRVAL_P(paths), pathsOffset+1);
	THUNDER_G(actionName) = zend_string_tolower(Z_STR_P(field));

	zend_string *controllerPath;
	controllerPath = strpprintf(0, "%s%s%c%s.php", ZSTR_VAL(THUNDER_G(appRoot)), "controllers", DEFAULT_SLASH, ZSTR_VAL(THUNDER_G(controllerName)));
	
	char *c_path = ZSTR_VAL(controllerPath);

	//加载执行controller文件
	int flag;
	flag = zend_execute_scripts_ext(c_path);


	if(flag == FAILURE){

		zend_error_noreturn(E_CORE_ERROR,"Couldn't find file: %s.",c_path);

	}

      

	//查找controller对应的
	//zend_class_entry *zend_lookup_class(zend_string *name);
	zend_class_entry *controller_ce = zend_lookup_class(THUNDER_G(controllerName));

	if(controller_ce == NULL){

		zend_error_noreturn(E_CORE_ERROR,"Couldn't find file: %s.",c_path);
	}


	zval obj;
	object_init_ex(&obj, controller_ce);

	
	zval function_name;
	ZVAL_STRING(&function_name,ZSTR_VAL(THUNDER_G(actionName)));

	
	flag = call_user_class_method(return_value, controller_ce, &obj, function_name, 0, NULL);

	if(flag == FAILURE){


		zend_error_noreturn(E_CORE_ERROR, 
							"Couldn't find implementation for method %s%s%s", 
							controller_ce ? ZSTR_VAL(controller_ce->name) : "", 
							controller_ce ? "::" : "", 
							function_name);
	
	}
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(thunder)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(thunder)
{
	THUNDER_G(uri) = NULL;
	THUNDER_G(controllerName) = NULL;
	THUNDER_G(actionName) = NULL;
	array_init(&THUNDER_G(paths));
#if defined(COMPILE_DL_THUNDER) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(thunder)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(thunder)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "thunder support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */



/* {{{ thunder_module_entry
 */
zend_module_entry thunder_module_entry = {
	STANDARD_MODULE_HEADER,
	"thunder",
	thunder_functions,
	PHP_MINIT(thunder),
	PHP_MSHUTDOWN(thunder),
	PHP_RINIT(thunder),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(thunder),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(thunder),
	PHP_THUNDER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_THUNDER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(thunder)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
