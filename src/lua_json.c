#include "lua_json.h"
//#define DUMPSTACK 1

#ifdef DUMPSTACK
static void dumpstack(lua_State *L, const char *msg)
{
	printf("************** begin %s ****************\n", msg);
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++)
	{
		printf("%d\t%s\t", i, luaL_typename(L, i));
		switch (lua_type(L, i))
		{
		case LUA_TNUMBER:
			printf("%g\n", lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			printf("%s\n", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			printf("%s\n", "nil");
			break;
		default:
			printf("%p\n", lua_topointer(L, i));
			break;
		}
	}
	printf("**********************************\n");
}
#endif

const char *Json[] = {
    "\"%s\":",      // JsonKey      0
    "\"%s\":\"%s\"",// ObjString    1
    "\"%s\":%f",    // ObjNumber    2
    "\"%s\":%s",    // ObjBool      3
    "\"%s\":%s",    // ObjNull      4
    "\"%s\"",       // ArrString    5
    "%f",           // ArrNumber    6
    "%s",           // ArrBool      7
    "%s",           // ArrNull      8
    ",",            // JsonNext     9
    "{",            // OpenObj      10
    "}",            // CloseObj     11
    "[",            // OpenArr      12
    "]"             // CloseArr     13
};

const char *fields[] = {
	"children",
	"ids",
	"keys",
	"klens",
	"vlens",
	"vtypes"
};

json_elm *check_json_elm(lua_State *L, int pos) {
	json_elm *e = (json_elm*)lua_topointer(L, pos);
	void *ud = NULL; 
	
	if(!e) { // if e is NULL force a lua error and bailout
		ud = luaL_checkudata(L, pos, "JSON.json");
		luaL_argcheck(L, ud != NULL, pos, "`json element' expected");
	}		

	switch(e->type){
		case JSON_ARRAY_TYPE: {
			ud = luaL_checkudata(L, pos, "JSON.array");
    		luaL_argcheck(L, ud != NULL, pos, "`json array' expected");
			break;
		}
		case JSON_OBJECT_TYPE: {
			ud = luaL_checkudata(L, pos, "JSON.object");
    		luaL_argcheck(L, ud != NULL, pos, "`json object' expected");
			break;
		}
		default: {
			ud = luaL_checkudata(L, pos, "JSON.json");
    		luaL_argcheck(L, ud != NULL, pos, "`json element' expected");
			break;
		}
	}

	return (json_elm*)ud;
};

bool check_next(ref *seen, uintptr_t next) {;
	if(DEBUG) printf("rlen %ld root: %ld elm: %ld nested: %ld\n", seen->rlen, seen->root, seen->elm->id, seen->nested->id);
	seen->elm->recurs++;
	if(next == seen->root) {
		if(++seen->depth > seen->max) 
			return true;
		else {
			seen->rlen += seen->nested->rlen;


			if(DEBUG) printf("seen elm: %ld  seen nested: %ld seen rlen %ld\n", seen->elm->rlen, seen->nested->rlen, seen->rlen);
			seen->b = realloc(seen->b, seen->rlen);
			//seen->rlen = p;
		}
	}
	seen->last = next;
	
	return false;
};

size_t __lua_json_elm_get_rlen(json_elm *elm, int mode, bool esc) {
	size_t rlen = 0;
	switch(mode) {
		// json
		case 0:
			rlen += esc ? (elm->rlen + (elm->quoted * 2) + (elm->nkeys * 2))
			    	    : (elm->rlen);

			//rlen = esc ? (rlen - (rlen *0.0111)) : rlen; // ToDO count of escaped json is out by 2/100 charactes
		
			break;
		// lua	
		case 1:
			rlen += esc ? ((elm->rlen + (elm->quoted * 2)) - (elm->nkeys * 2))
				        : (elm->rlen - (elm->nkeys * 2));
			break;
		// c_str		
	}

	return rlen;
}

int
lua_json_elm_get_rlen(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	int rtype = luaL_checkinteger(L, 2);
	bool esc = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

	size_t rlen = __lua_json_elm_get_rlen(elm, rtype, esc);

	lua_pushnumber(L, rlen);

	return 1;
}

// init a new env manager 
void lua_json_env_manager_init(json_elm *elm, elm_env *env) {
	// array 
	env->elm 		= elm;
	env->add 		= &lua_json_elm_env_add;
	env->insert 		= &lua_json_elm_env_insert;
	env->get		= &lua_json_elm_env_get;
	env->rem		= &lua_json_elm_env_rem;
	
}

int
lua_json_elm_env_get(lua_State *L, json_elm *elm , int idx, env_field field) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	printf("---------------------------------------------------->>> field[%s]\n", fields[field]);
	lua_getfield(L, -1, fields[field]);
	size_t size = lua_objlen(L, -1);
	if(idx > (int)size) {
		fprintf(stderr, "Index [%d] is out of bounds !!", idx);
		lua_pop(L, 2);

		return 0; 
	}

	lua_rawgeti(L, -1, idx);
	//lua_insert(L, 2);
	//lua_pop(L, (lua_gettop(L)-2));
	//dumpstack(L);
	return 1;
}

int
lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field) {
	//dumpstack(L, "env_add");
	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	// stack { ..., vtable }
	lua_getfield(L, -1, fields[field]);
	// stack { ..., vtable, ids }
	size_t size = lua_objlen(L, -1);
	field == keys ? lua_pushstring(L, val->key) : lua_pushinteger(L, val->num);
	// stack { ..., vtable, ids, id }
	lua_rawseti(L, -2, size + 1);
	// stack { ..., vtable, ids }
	lua_pop(L, 2);
	// stack { ... }
	//if(DEBUG) {
		field == keys ? printf("%s\n", lua_pushfstring(L, "Added %s to %s[%d]", val->key, fields[field], (int)(size + 1)))
			      : printf("%s\n", lua_pushfstring(L, "Added %d to %s[%d]", val->num, fields[field], (int)(size + 1)));
		// stack { ..., fstring}
		lua_pop(L, 1);
		// stack { ... }
	//}
	// stack { ... }
	//dumpstack(L, "env_add_end");
	return 0;
}

int
lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field) {
	// stack {..., elm }

	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	// stack {..., elm, vtable }
	lua_getfield(L, -1, fields[field]);
	// stack {..., elm, vtable, ids }
	if(idx < 1 || idx > (int)elm->nelms+1) {
		fprintf(stderr, "Insert Id Index %d is out of range\n", idx);

		return 0;
	}

	for (int i = (int)elm->nelms; i >= idx; i--) {
		lua_rawgeti(L, -1, i);     				// Get element at i
		// stack {..., elm, vtable, ids, val[i] }
		lua_rawseti(L, -2, i + 1); 				// Move it to i + 1
		// stack {..., elm, vtable, ids }
	}
	// stack {..., elm, vtable, ids }
	field == keys ? lua_pushstring(L, val->key) : lua_pushinteger(L, val->num);
	// stack {..., elm, vtable, ids, id }
	lua_rawseti(L, -2, idx);
	// stack {..., elm, vtable, ids }
	lua_pop(L, 2);
	// stack {..., elm }
	return 0;
}

int
lua_json_elm_env_rem(lua_State *L, json_elm *elm, int idx, env_field field) {
	// stack {..., elm }
	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	// stack {..., elm, vtable }
	lua_getfield(L, -1, fields[field]);
	// stack {..., elm, vtable, ids }
	size_t size = lua_objlen(L, -1);

	for (int i = idx; i <= (int)size; i++) {
		lua_rawgeti(L, -1, i + 1); 			// Get element at i + 1
		// stack { ..., elm, vtable, ids val[i + 1] }
		lua_rawseti(L, -2, i); 	   			// Move it to i 
		// stack { ..., elm, vtable }

	}
	// stack { ..., elm, vtable, ids }
	lua_pop(L, 2);
	// stack {..., elm }
	return 0;
};

int
lua_json_elm_stringify(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);

	ref seen = {0};
	seen.elm = elm;
	seen.max = 1;
	seen.depth = 0;
	seen.marshal = lua_json_marshall_new(L);
	seen.root = seen.last = (uintptr_t)lua_topointer(L, -1);
	seen.check_next = &check_next;
	seen.mode = elm->mode;
	seen.escape = elm->escape;
	seen.Marshal = elm->mode == MARSHAL_JSON ? marshal_json : marshal_lua;
	seen.rlen = __lua_json_elm_get_rlen(elm, elm->mode, elm->escape);
	seen.ltype = LUA_TUSERDATA;
	seen.b = malloc(seen.rlen + 1); 
	memset(seen.b, 0, (seen.rlen + 1) );
	
	elm->render(L, &seen);

	size_t lt = strlen(seen.b)+1; // null term
	seen.b[lt]='\0';
	lua_pushstring(L, seen.b);
	free(seen.b);
	free(seen.marshal);

	if(DEBUG) {
		printf("\n\n########### result ###########\n\n");
		printf("mode: %s\nescape: %s\n", elm->mode == MARSHAL_JSON ? "json" : "lua", btoa(elm->escape));
		printf("marhsaled len: %ld\nrender len: %ld\n", lt, seen.rlen);
		printf("\n########### %ld/%ld ###########\n\n", lt, seen.rlen);

		printf("JSON CSTR RENDER LENGTH:\t%ld\nLUA CSTR RENDER LENGTH:\t\t%ld\nJSON RENDER LENGTH:\t\t%ld\nLUA RENDER LENGTH:\t\t%ld\n\n",
													__lua_json_elm_get_rlen(elm, 0, true),
													__lua_json_elm_get_rlen(elm, 1, true),
													__lua_json_elm_get_rlen(elm, 0, false),
													__lua_json_elm_get_rlen(elm, 1, false)
		);
	}
	return 1;

};

int
lua_json_elm_tostring(lua_State *L) {
	//dumpstack(L, "elm_tostring");
	json_elm *elm = check_json_elm(L, 1);
	if(elm)
		lua_pushfstring(L, "%s: %p", elm->typename, elm->id);
		else
			fprintf(stderr, "To String Error !!!!!\n");

    return 1;
};

int
lua_json_elm_len(lua_State *L) {
    json_elm *elm = check_json_elm(L, 1);
    if(elm) {
        lua_pushinteger(L, elm->rlen);
	}

    return 1;
};

int
lua_json_elm_size(lua_State *L) {
    json_elm *elm = check_json_elm(L, 1);
    if(elm) {
        lua_pushinteger(L, elm->nelms);
	}

    return 1;
};

void
json_type(parse_elm *elm) {
	const char index = elm->index;
	if (index == '{') {
		elm->type = JSON_OBJECT_TYPE;
		return;
	}
	if (index == '[') {
		elm->type = JSON_ARRAY_TYPE;
		return;
	}
	
	bool isLong = ((elm->l = _lua_json_get_long(*elm->json, elm->key, -1)) == -1) ? false : true;
	bool isStr = ((elm->str = _lua_json_get_str(*elm->json, elm->key)) == NULL) ? false : true;
	bool isNum = _lua_json_get_num(*elm->json, elm->key, &elm->num);
	bool isBool = _lua_json_get_bool(*elm->json, elm->key, &elm->b);

	if(isNum)
		elm->type = JSON_NUMBER_TYPE;
	else if(isStr)
		elm->type = JSON_STRING_TYPE;
	else if(isBool)
		elm->type = JSON_BOOL_TYPE;
	else if(isLong)
		elm->type = JSON_LONG_TYPE;
	else
		elm->type = JSON_NULL_TYPE; 

	return;
};

bool _isRoot = true;

static int __lua_json_elm_parse(lua_State *L, struct lua_str json, int depth) {
	_isRoot = depth == -1 ? true : false;
	json_elm *jelm = check_json_elm(L, -1);
	// stack {..., env }
	parse_elm elm = {0};
	int idx = 0;
	int n = 0, o = _lua_json_get(json, "$", &n);
	if(_isRoot) jelm->plen = n;
	if (json.buf[o] == '{' || json.buf[o] == '[') {
		struct lua_str key, val, sub = lua_str_n(json.buf + o, (size_t)n);
		size_t ofs = 0;
		while ((ofs = _lua_json_next(sub, ofs, &key, &val)) > 0) {
			bool isKeyed = key.len > 0 ? true : false;
			elm.isRoot = _isRoot;
			if(elm.isRoot) depth = 1;
			elm.index = val.buf[0];
			
			if (isKeyed) { 
				lua_pushlstring(L, key.buf + 1, key.len - 2);
				elm.key = lua_pushfstring(L, "$.%s", lua_tostring(L, -1));
				lua_pop(L, 2);

				elm.json = &json;
				json_type(&elm);

			}
			else {
				// construct a json object with a dummy key and the value to check type
				lua_pushlstring(L, val.buf, (int)val.len);
				const char *buf = lua_pushfstring(L, "{\"%s\": %s}", "dkey", lua_tostring(L, -1));
				lua_pop(L, 2); 
				//dumpstack(L);
				struct lua_str djson = lua_str(buf);
				elm.json = &djson;
				elm.key = "$.dkey";
				json_type(&elm);	
			}
			switch (elm.type) {
				case JSON_NUMBER_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushnumber(L, elm.num);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushnumber(L, elm.num);
					}
					//printf("JSON NUMBER\n");
					//dumpstack(L);
					jelm->nums++;
					lua_settable(L, -3);
					break;
				}
				case JSON_BOOL_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushboolean(L, elm.b);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushboolean(L, elm.b);
					}
					//printf("JSON BOOL\n");
					//dumpstack(L);
					jelm->bools++;
					lua_settable(L, -3);
					break;
				}
				case JSON_LONG_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushnumber(L, elm.l);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushnumber(L, elm.l);
					}
					//printf("JSON LONG\n");
					//dumpstack(L);
					jelm->longs++;
					lua_settable(L, -3);
					break;
				}
				case JSON_STRING_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushstring(L, elm.str);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushstring(L, elm.str);
					}
					free(elm.str);
					//printf("JSON STRING\n");
					//dumpstack(L);
					jelm->strings++;
					lua_settable(L, -3);
					break;
				}
				case JSON_NULL_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushlstring(L, val.buf, val.len);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushlstring(L, val.buf, val.len);
					}
					//printf("JSON BOOL\n");
					//dumpstack(L);
					jelm->nulls++;
					lua_settable(L, -3);
					break;
				}
				case JSON_ARRAY_TYPE: {
					//printf("NEW OBJECT !!!!!!!!!!!!!!!!\n");
					isKeyed ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushnumber(L, idx++);
					lua_json_elm_parse_array(L);
					jelm->arrays++;
					break;
				}
				case JSON_OBJECT_TYPE: {
					//printf("NEW OBJECT !!!!!!!!!!!!!!!!\n");
					if (isKeyed){
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_json_elm_parse_object(L);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_json_elm_parse_object(L);
					}
					jelm->objects++;
					break;
				}
				break;
			}
			// found nested object or array
			if (*val.buf == '[' || *val.buf == '{') {
				switch (*val.buf) {
					case '[': {
						//jelm->arrays++;
						jelm->nestedarrays++;
						break;
					}
					case '{': {
						jelm->nestedobjects++;	
						break;
					}
				}
				__lua_json_elm_parse(L, val, depth += 1);
			}
			// found the end of the current object / array
			char last = json.buf[ofs];
			if (last == ']' || last == '}') {

				// we're finished parsing when stack size is 1
				bool finished = lua_gettop(L) == 1 ? true : false;

				if (!finished) {
					lua_settable(L, -3);
					depth -= 1;
					idx = 0;
				}
			}
		}
	}
	// all done!! leave the table on the stack and return it
	if (depth == 1) {
		return 1;
	}
	// recursion
	return 0;
};

int
lua_json_elm_parse(lua_State *L) {
	const char *elm = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	//printf("JSON: %s\n", elm);
	if(elm[0] == '{') 
		lua_json_elm_parse_object(L);
	else if(elm[0] == '[')
		lua_json_elm_parse_array(L);
	else {
		fprintf(stderr, " (_lua_json_elm_parse) Ivalid Json Element\n");
		return 0;
	}

	struct lua_str json = lua_str(elm); 
	__lua_json_elm_parse(L, json, -1);

	//json_elm *jelm = check_json_elm(L, 1);

	//printf("PARSED ELM TYPE: %s\nPARSED LENGTH: %ld\n", jelm->typename, jelm->rlen);
	
	return 1;
};

int
lua_json_elm_info(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(lua_toboolean(L, 2) != 1) {
		lua_pushnumber(L, elm->nelms);
		lua_pushstring(L, elm->typename);

		return 2;
	}
	else
		lua_pushfstring(L, "Elm Type: %s Elm Size: %d", elm->typename, (int)elm->nelms);

	return 1;
};

// need to refactor to use evn_val instead of void *
void alloc_events(json_elm *elm) {
	elm->event = (elm_events*)malloc(sizeof(elm_events));
	memset(elm->event, 0, sizeof(elm_events));

	elm->event->on_newindex = (Subject*)malloc(sizeof(Subject));
	memset(elm->event->on_newindex, 0, sizeof(*elm->event->on_newindex));

    	elm->event->on_change = (Subject*)malloc(sizeof(Subject));
    	memset(elm->event->on_change, 0, sizeof(*elm->event->on_change));
	
	elm->event->init 		= &subject_init;
	elm->event->sub 		= &subject_subscribe;
	elm->event->set 		= &subject_set_values;
	elm->event->unsub 		= &subject_unsubscribe;
	elm->event->cleanup 		= &subject_cleanup;

	elm->event->init(elm->event->on_newindex);
	elm->event->init(elm->event->on_change);

	return;
}

static const struct luaL_reg lua_json_methods[] = {
	{ "parse",	    	lua_json_elm_parse	},
	{ "parse_lua",	    	lua_json_lua_parse	},
	{ "stringify",		lua_json_elm_stringify	},
	{ "stringify_lua",	lua_json_lua_stringify	},
	{ "len",		lua_json_elm_len	},
	{ "table_len",		lua_json_lua_table_len	},
	{ "table_type",		lua_json_lua_is_mixed	},
	{ "__tostring",		lua_json_elm_tostring	},
	//{ "tojson",		lua_json_elm_tojson	},
    	{ "__len",		lua_json_elm_size	},
	{NULL, NULL}
};

int
luaopen_JSON(lua_State *L){
	luaL_newmetatable(L, "JSON.json");
	lua_pushvalue(L, -1);	// pushes the metatable
	lua_setfield(L, -2, "__index");	// metatable.__index = metatable
	luaL_register(L, LUA_JSON, lua_json_methods);
	
	lua_pushliteral(L, null);
	lua_setglobal(L, "null");

    lua_json_open_array(L);
    lua_json_open_object(L);

    return 1;
};
