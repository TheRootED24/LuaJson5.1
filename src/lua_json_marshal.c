#include "lua_json_marshal.h"

/*static void dumpstack(lua_State *L)
{
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
};*/

const char *marshal_json[] = {
    "\"%s\":",      		// ObjKey      	0
    "\"%s\":\"%s\"",		// ObjString    1
    "\"%s\":%f",    		// ObjNumber    2
    "\"%s\":%s",    		// ObjBool      3
    "\"%s\":%s",    		// ObjNull      4
    "\"%s\"",       		// ArrString    5
    "%f",           		// ArrNumber    6
    "%s",           		// ArrBool      7
    "%s",           		// ArrNull      8
    ",",            		// JsonNext     9
    "{",            		// OpenObj      10
    "}",            		// CloseObj     11
    "[",            		// OpenArr      12
    "]",            		// CloseArr     13
    "\\\"%s\\\":\\\"%s\\\"", 	// EscObjString 14
    "\\\"%s\\\"" ,      	// EscArrString 15
    "\\\"%s\\\":",		// EscObjKey    16
    "\\\"%s\\\":%f",    	// EscObjNumber 17
    "\\\"%s\\\":%s",    	// EscObjBool   18
    "\\\"%s\\\":%s"    		// EscObjNull   19
};

const char *marshal_lua[] = {
    "%s=",			// ObjKey      	0
    "%s=\"%s\"",		// ObjString    1
    "%s=%f",			// ObjNumber    2
    "%s=%s",			// ObjBool      3
    "%s=%s",			// ObjNull      4
    "\"%s\"",			// ArrString	5
    "%f",			// ArrNumber    6
    "%s",			// ArrBool      7
    "%s",			// ArrNull      8
    ",",			// Next     	9
    "{",			// OpenObj      10
    "}",			// CloseObj     11
    "[",			// OpenArr      12
    "]",			// CloseArr     13
    "%s=\\\"%s\\\"",		// EscObjString 14
    "\\\"%s\\\"",		// EscArrString 15
    "\\\"%s\\\":",		// EscObjKey    16
    "\\\"%s\\\":%f",    	// EscObjNumber 17
    "\\\"%s\\\":%s",    	// EscObjBool   18
    "\\\"%s\\\":%s"    		// EscObjNull   19

};

static int 
marshal_next(ref *seen)
{
	strcat(seen->b, seen->Marshal[Next]);

	return 0;
};

// ****************************** MARSHAL OBJECT ELEMENT ******************************************

static int
marshal_object_open(ref *seen)
{
	strcat(seen->b, seen->Marshal[OpenObj]);

	return 0;
};


static int
marshal_object_close(ref *seen)
{
	strcat(seen->b, seen->Marshal[CloseObj]);

	return 0;
};


static int
marshal_object_key(lua_State *L, json_elm *elm, ref *seen)
{
	//dumpstack(L);
	if(seen->escape && seen->mode == MARSHAL_JSON)
		strcat(seen->b, lua_pushfstring(L, seen->Marshal[EscObjKey], elm->key));
	else
		strcat(seen->b, lua_pushfstring(L, seen->Marshal[ObjKey], elm->key));

	lua_pop(L, 1);

	return 0;
};

static int
marshal_object_string(lua_State *L, json_elm *elm, ref *seen)
{
	elm->val = luaL_checklstring(L, -1, &elm->vlen);

	if ( elm->vlen  == 4 && strcmp(elm->val, "null") == 0) 
		if(seen->escape && seen->mode == MARSHAL_JSON)
			strcat(seen->b, lua_pushfstring(L, seen->Marshal[EscObjNull], elm->key, elm->val));
		else
			strcat(seen->b, lua_pushfstring(L, seen->Marshal[ObjNull], elm->key, elm->val));
	else 
		if(seen->escape)
			strcat(seen->b, lua_pushfstring(L, seen->Marshal[EscObjString], elm->key, elm->val));
		else
			strcat(seen->b, lua_pushfstring(L, seen->Marshal[ObjString], elm->key, elm->val));

	lua_pop(L, 1);

	return 0;
};

static int
marshal_object_number(lua_State *L, json_elm *elm, ref *seen)
{
	lua_pushvalue(L, -1);

	seen->escape && seen->mode == MARSHAL_JSON ? strcat(seen->b, lua_pushfstring(L, seen->Marshal[EscObjNumber], elm->key, luaL_checknumber(L, -1)))
		     				   				   : strcat(seen->b, lua_pushfstring(L, seen->Marshal[ObjNumber], elm->key, luaL_checknumber(L, -1)));

	lua_pop(L, 2);

	return 0;				
};

static int
marshal_object_bool(lua_State *L, json_elm *elm, ref *seen)
{
	lua_pushvalue(L, -1);

	seen->escape && seen->mode == MARSHAL_JSON ? strcat(seen->b, lua_pushfstring(L, seen->Marshal[EscObjBool], elm->key, luaL_checknumber(L, -1)))
		     				   				   : strcat(seen->b, lua_pushfstring(L, seen->Marshal[ObjBool], elm->key, btoa(lua_toboolean(L, -1))));

	lua_pop(L, 2);

	return 0;				
};


// ****************************** MARSHAL ARRAY ELEMENT ******************************************
static int
marshal_array_open(ref *seen)
{
	strcat(seen->b, seen->Marshal[OpenArr]);

	return 0;
};

static int
marshal_array_close(ref *seen)
{
	strcat(seen->b, seen->Marshal[CloseArr]);

	return 0;
};

static int 
marshal_array_string(lua_State *L, json_elm *elm, ref *seen)
{
	elm->val = luaL_checklstring(L, -1, &elm->vlen);

	if (elm->vlen  == 4 && strcmp(elm->val, "null") == 0)
		strcat(seen->b, lua_pushfstring(L, seen->Marshal[ArrNull], elm->val));
	else 
		if(seen->escape)
			strcat(seen->b,lua_pushfstring(L, seen->Marshal[EscArrString], elm->val));
		else
			strcat(seen->b,lua_pushfstring(L, seen->Marshal[ArrString], elm->val));
	
	lua_pop(L, 1);

	return 0;
};

static int
marshal_array_number(lua_State *L, ref *seen)
{
	lua_pushvalue(L, -1);
	strcat(seen->b, lua_pushfstring(L, seen->Marshal[ArrNumber], luaL_checknumber(L, -1)));

	lua_pop(L, 2);

	return 0;				
};

static int
marshal_array_bool(lua_State *L, ref *seen)
{
	lua_pushvalue(L, -1);
	strcat(seen->b, lua_pushfstring(L, seen->Marshal[ArrBool], btoa(lua_toboolean(L, -1))));

	lua_pop(L, 2);

	return 0;				
};

marshal* lua_json_marshall_new() {
	marshal *m = (marshal*)malloc(sizeof(marshal));
	memset(m, 0, sizeof(marshal));

	// c side methods
	m->mode 		= marshal_json;
	m->escape		= false;
	m->next			= &marshal_next;
	// object methods
	m->obj_key 		= &marshal_object_key;
	m->obj_open		= &marshal_object_open;
	m->obj_close 	= &marshal_object_close;
	m->obj_string 	= &marshal_object_string;
	m->obj_number 	= &marshal_object_number;
	m->obj_bool 	= &marshal_object_bool;
	// array metods
	m->arr_open		= &marshal_array_open;
	m->arr_close 	= &marshal_array_close;
	m->arr_string 	= &marshal_array_string;
	m->arr_number 	= &marshal_array_number;
	m->arr_bool 	= &marshal_array_bool;

	return m;
};
