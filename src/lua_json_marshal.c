#include "lua_json_marshal.h"

const char *marshal_json[] = {
    "\"%s\":",		     // ObjKey      	0
    "\"%s\":\"%s\"",	     // ObjString    1
    "\"%s\":%f",	     // ObjNumber    2
    "\"%s\":%s",	     // ObjBool      3
    "\"%s\":%s",	     // ObjNull      4
    "\"%s\"",		     // ArrString    5
    "%f",		     // ArrNumber    6
    "%s",		     // ArrBool      7
    "%s",		     // ArrNull      8
    ",",		     // JsonNext     9
    "{",		     // OpenObj      10
    "}",		     // CloseObj     11
    "[",		     // OpenArr      12
    "]",		     // CloseArr     13
    "\\\"%s\\\":\\\"%s\\\"", // EscObjString 14
    "\\\"%s\\\"",	     // EscArrString 15
    "\\\"%s\\\":",	     // EscObjKey    16
    "\\\"%s\\\":%f",	     // EscObjNumber 17
    "\\\"%s\\\":%s",	     // EscObjBool   18
    "\\\"%s\\\":%s"	     // EscObjNull   19
};

const char *marshal_lua[] = {
    "%s=",	     // ObjKey      	0
    "%s=\"%s\"",     // ObjString    1
    "%s=%f",	     // ObjNumber    2
    "%s=%s",	     // ObjBool      3
    "%s=%s",	     // ObjNull      4
    "\"%s\"",	     // ArrString	5
    "%f",	     // ArrNumber    6
    "%s",	     // ArrBool      7
    "%s",	     // ArrNull      8
    ",",	     // Next     	9
    "{",	     // OpenObj      10
    "}",	     // CloseObj     11
    "[",	     // OpenArr      12
    "]",	     // CloseArr     13
    "%s=\\\"%s\\\"", // EscObjString 14
    "\\\"%s\\\"",    // EscArrString 15
    "\\\"%s\\\":",   // EscObjKey    16
    "\\\"%s\\\":%f", // EscObjNumber 17
    "\\\"%s\\\":%s", // EscObjBool   18
    "\\\"%s\\\":%s"  // EscObjNull   19

};

static int
marshal_next(ref *seen)
{
	// strcat(seen->b, seen->Marshal[Next]);
	luaL_addstring(seen->B, seen->Marshal[Next]);

	return 0;
};

// ****************************** MARSHAL OBJECT ELEMENT ******************************************

static int
marshal_object_open(ref *seen)
{
	// strcat(seen->b, seen->Marshal[OpenObj]);
	luaL_addstring(seen->B, seen->Marshal[OpenObj]);

	return 0;
};

static int
marshal_object_close(ref *seen)
{
	// strcat(seen->b, seen->Marshal[CloseObj]);
	luaL_addstring(seen->B, seen->Marshal[CloseObj]);
	return 0;
};

static int
marshal_object_key(lua_State *L, json_elm *elm, ref *seen)
{
	// lua_pushfstring pushes the result and returns a pointer to it.
	if (seen->escape && seen->mode == MARSHAL_JSON)
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscObjKey], elm->key));
	else
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ObjKey], elm->key));

	// 4. Cleanup
	// Pop the formatted string from L to keep the stack neutral.
	lua_pop(L, 1);
	seen->nkeys++;

	return 0;
}

static int marshal_object_string(lua_State *L, json_elm *elm, ref *seen)
{
	// Retrieve the string pointer from the stack (Index -1)
	elm->val = luaL_checklstring(L, -1, &elm->vlen);

	if (elm->val == NULL_CACHE)
	{
		if (seen->escape && seen->mode == MARSHAL_JSON)
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscObjNull], elm->key, elm->val));
		else
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ObjNull], elm->key, elm->val));
	}
	else
	{
		if (seen->escape)
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscObjString], elm->key, elm->val));
		else
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ObjString], elm->key, elm->val));

		seen->quoted++;
	}

	seen->nkeys++;
	// Clean up: Pop the formatted string pushed by lua_pushfstring
	lua_pop(L, 1);

	return 0;
}

static int marshal_object_number(lua_State *L, json_elm *elm, ref *seen)
{
	lua_pushvalue(L, -1); // Stack: [..., val, val_copy]

	if (seen->escape && seen->mode == MARSHAL_JSON)
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscObjNumber], elm->key, luaL_checknumber(L, -1)));
	else
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ObjNumber], elm->key, luaL_checknumber(L, -1)));

	// Stack: [..., val, val_copy, fmt_string]
	lua_pop(L, 2); // Pop val_copy and fmt_string. Stack: [..., val]
	seen->nkeys++;

	return 0;
}

static int marshal_object_bool(lua_State *L, json_elm *elm, ref *seen)
{
	lua_pushvalue(L, -1); // Stack: [..., val, val_copy]

	if (seen->escape && seen->mode == MARSHAL_JSON)
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscObjBool], elm->key, btoa(lua_toboolean(L, -1))));
	else
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ObjBool], elm->key, btoa(lua_toboolean(L, -1))));

	// Stack: [..., val, val_copy, fmt_string]
	lua_pop(L, 2);
	seen->nkeys++;

	return 0;
}

static int marshal_array_open(ref *seen)
{
	// Replace strcat with Buffer Append
	luaL_addstring(seen->B, seen->Marshal[OpenArr]);
	return 0;
}

static int marshal_array_close(ref *seen)
{
	// Replace strcat with Buffer Append
	luaL_addstring(seen->B, seen->Marshal[CloseArr]);
	return 0;
}

static int
marshal_array_string(lua_State *L, json_elm *elm, ref *seen)
{
	// 1. Get pointer to source string
	elm->val = luaL_checklstring(L, -1, &elm->vlen);

	// 2. Format and Append
	if (elm->val == NULL_CACHE)
		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrNull], elm->val));
	else
	{
		if (seen->escape)
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscArrString], elm->val));
		else
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrString], elm->val));

		seen->quoted++;
	}

	// 3. Cleanup: Pop the formatted string from stack
	lua_pop(L, 1);

	return 0;
}

static int marshal_array_number(lua_State *L, ref *seen)
{
	lua_pushvalue(L, -1); // Stack: [..., val, val_copy]

	// Append formatted number to Buffer
	luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrNumber], luaL_checknumber(L, -1)));

	// Pop val_copy and the string pushed by lua_pushfstring
	lua_pop(L, 2);

	return 0;
}

static int marshal_array_bool(lua_State *L, ref *seen)
{
	lua_pushvalue(L, -1); // Stack: [..., val, val_copy]

	// Append formatted boolean to Buffer
	luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrBool], btoa(lua_toboolean(L, -1))));

	// Pop val_copy and the string pushed by lua_pushfstring
	lua_pop(L, 2);

	return 0;
}

marshal *lua_json_marshall_new()
{
	marshal *m = (marshal *)malloc(sizeof(marshal));
	memset(m, 0, sizeof(marshal));

	// c side methods
	m->mode = marshal_json;
	m->escape = false;
	m->next = &marshal_next;
	// object methods
	m->obj_key = &marshal_object_key;
	m->obj_open = &marshal_object_open;
	m->obj_close = &marshal_object_close;
	m->obj_string = &marshal_object_string;
	m->obj_number = &marshal_object_number;
	m->obj_bool = &marshal_object_bool;
	// array metods
	m->arr_open = &marshal_array_open;
	m->arr_close = &marshal_array_close;
	m->arr_string = &marshal_array_string;
	m->arr_number = &marshal_array_number;
	m->arr_bool = &marshal_array_bool;

	return m;
};