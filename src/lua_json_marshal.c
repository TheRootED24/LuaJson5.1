/*
 * LuaJson - a json api library for Lua
 *
 *   Copyright (C) 2026 TheRootED24 <TheRootED24@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in JSONliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "lua_json_marshal.h"

const char *marshal_json[] = {
    "\"%s\":",		  		 // ObjKey       0
    "\"%s\":\"%s\"",  		 // ObjString    1
    "\"%s\":%f",	  		 // ObjNumber    2
    "\"%s\":%d",	  		 // ObjInteger   3
    "\"%s\":%s",	  		 // ObjBool      4
    "\"%s\":%s",	  		 // ObjNull      5
    "\"%s\"",		  		 // ArrString    6
    "%f",		      		 // ArrNumber    7
    "%d",		      		 // ArrInteger   8
    "%s",		      		 // ArrBool      9
    "%s",		      		 // ArrNull      10
    ",",		      		 // JsonNext     11
    "{",		      		 // OpenObj      12
    "}",		      		 // CloseObj     13
    "[",		      		 // OpenArr      14
    "]",		      		 // CloseArr     15
    "\\\"%s\\\":\\\"%s\\\"", // EscObjString 16
    "\\\"%s\\\"",	         // EscArrString 17
    "\\\"%s\\\":",	         // EscObjKey    18
    "\\\"%s\\\":%f",	     // EscObjNumber 19
    "\\\"%s\\\":%d",	     // EscObjInteger20
    "\\\"%s\\\":%s",	     // EscObjBool   21
    "\\\"%s\\\":%s"	         // EscObjNull   22
	"\\\"%s\\\":%s",	     // EscObjInt64  23
	"\"%s\":%s",	  		 // ObjInteger64 24
	"%s",		      		 // ArrInteger64 25
};

const char *marshal_lua[] = {
    "%s=",	     			 // ObjKey       0
    "%s=\"%s\"",     		 // ObjString    1
    "%s=%f",	     		 // ObjNumber    2
    "%s=%d",	     		 // ObjInteger   3
    "%s=%s",	     		 // ObjBool      4
    "%s=%s",	     		 // ObjNull      5
    "\"%s\"",	     		 // ArrString    6
    "%f",	     			 // ArrNumber    7
    "%d",	     			 // ArrInteger   8
    "%s",	     			 // ArrBool      9
    "%s",	     			 // ArrNull      10
    ",",	     			 // Next         11
    "{",	     			 // OpenObj      12
    "}",	     			 // CloseObj     13
    "[",	     			 // OpenArr      14
    "]",	     			 // CloseArr     15
    "%s=\\\"%s\\\"", 		 // EscObjString 16
    "\\\"%s\\\"",    		 // EscArrString 17
    "\\\"%s\\\":",   		 // EscObjKey    18
    "\\\"%s\\\":%f", 		 // EscObjNumber 19
    "\\\"%s\\\":%d", 		 // EscObjInteger20
    "\\\"%s\\\":%s", 		 // EscObjBool   21
    "\\\"%s\\\":%s"  		 // EscObjNull   22
	"\\\"%s\\\":%d", 		 // EscObjIn64   23
	"%s=%s",	     		 // ObjInteger64 24
	"%s",	     			 // ArrInteger64 25


};

const char *marshal_bash[] = {
    "[%s]=",                  // ObjKey       0  -> e.g., [name]=
    "[%s]=\"%s\"",            // ObjString    1  -> e.g., [name]="value"
    "[%s]=%f",                // ObjNumber    2  
    "[%s]=%d",                // ObjInteger   3  
    "[%s]=%s",                // ObjBool      4  -> e.g., [isAdmin]=true
    "[%s]=%s",                // ObjNull      5  -> Empty string for null
    "\\\"%s\\\"",                 // ArrString    6  
    "%f",                     // ArrNumber    7  
    "%d",                     // ArrInteger   8  
    "%s",                     // ArrBool      9  
    "%s",                     // ArrNull      10 
    " ",                      // Next         11 -> Bash uses spaces/newlines instead of commas!
    "(",                      // OpenObj      12 -> e.g., declare -A my_dict=(
    ")",                      // CloseObj     13 -> )
    "(",                      // OpenArr      14 -> e.g., declare -a my_list=(
    ")",                      // CloseArr     15 -> )
    "[%s]=\\\"%s\\\"",        // EscObjString 16 
    "\\\"%s\\\"",             // EscArrString 17 
    "[%s]=",                  // EscObjKey    18 -> (Depends on how your escaping handles brackets)
    "[%s]=%f",                // EscObjNumber 19 
    "[%s]=%d",                // EscObjInteger20 
    "[%s]=%s",                // EscObjBool   21 
    "[%s]=%s",                // EscObjNull   22 
    "[%s]=%s",                // EscObjIn64   23 
    "[%s]=%s",                // ObjInteger64 24 
    "%s",                     // ArrInteger64 25
    "[%s]=\"",                // BashNestKey  26 -> Result: [arr]="
    "\"",                     // NestedArr    27 -> Result: " (for 2D array entries)
    ")\""                     // BashClose    28 -> Result: "
};


// BashNestKey  26
// NestedArr    27
// BashClose    28
static int marshal_next(ref *seen)
{
	// strcat(seen->b, seen->Marshal[Next]);
	luaL_addstring(seen->B, seen->Marshal[Next]);

	return 0;
};

// ****************************** MARSHAL OBJECT ELEMENT ******************************************

static int marshal_object_open(ref *seen)
{
	// strcat(seen->b, seen->Marshal[OpenObj]);
	    luaL_addstring(seen->B, seen->Marshal[OpenObj]);

	return 0;
};

static int marshal_object_close(ref *seen)
{
    
    if(seen->mode == MARSHAL_BASH && !seen->isRoot)
        luaL_addstring(seen->B, seen->Marshal[BashClose]);
    else
	    luaL_addstring(seen->B, seen->Marshal[CloseObj]);

	return 0;
};

static int marshal_object_key(lua_State *L, json_elm *elm, ref *seen)
{
	// lua_pushfstring pushes the result and returns a pointer to it.
    if(seen->mode == MARSHAL_BASH && !seen->isRoot)
        luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[BashKey], elm->key));
    else if (seen->escape && seen->mode == MARSHAL_JSON)
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
        if(seen->mode == MARSHAL_BASH) elm->val = BASH_NULL;

		if (seen->escape && seen->mode == MARSHAL_JSON)
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[EscObjNull], elm->key, elm->val));
		else
			luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ObjNull], elm->key, elm->val));

        seen->nulls++;
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

    double d = luaL_checknumber(L, -1);

    /* 1. SEPARATE INTEGERS FROM FLOATING-POINT DECIMALS LOCALLY IN C */
    if (d == (int64_t)d) {
        /* Format the 64-bit value into a string buffer in C to protect the bit width */
        char scratch[64];
        int len = snprintf(scratch, sizeof(scratch), "%" PRId64, (int64_t)d);
        lua_pushlstring(L, scratch, len); // Stack: [..., val, val_copy, digit_string]
        
        /* Look up the appropriate unquoted string passthrough format slot (ObjNull/EscObjNull) */
        const char *fmt = (seen->escape && seen->mode == MARSHAL_JSON) 
                          ? seen->Marshal[EscObjNull] 
                          : seen->Marshal[ObjNull];
                          
        /* Safely routes your key ("%s") and your raw pre-formatted digits string ("%s") */
        luaL_addstring(seen->B, lua_pushfstring(L, fmt, elm->key, lua_tostring(L, -1)));
        lua_pop(L, 3); // Balance stack neutral
        seen->nkeys++;
        return 0;
    }

    /* 2. DEFAULT FLOATING-POINT DECIMAL PATH (%f handles fractions natively) */
    const char *fmt = (seen->escape && seen->mode == MARSHAL_JSON) 
                      ? seen->Marshal[EscObjNumber] 
                      : seen->Marshal[ObjNumber];
                      
    luaL_addstring(seen->B, lua_pushfstring(L, fmt, elm->key, d));
    lua_pop(L, 2); 
    seen->nkeys++;
    return 0;
}

static int marshal_object_int64(lua_State *L, json_elm *elm, ref *seen)
{
    lua_pushvalue(L, -1); // Stack: [..., udata, udata_copy]

    /* Safely unbox the raw binary 8-byte integer out of the memory capsule */
    json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
    int64_t val = ud->value;

    /* Convert to text digits locally in C to completely bypass lua_pushfstring width bugs */
    char scratch[32];
    int len = snprintf(scratch, sizeof(scratch), "%" PRId64, val);
    lua_pushlstring(L, scratch, len); // Stack: [..., udata, udata_copy, digit_string]

    /* Resolve the explicit 64-bit object formatting slot strings */
    const char *fmt = (seen->escape && seen->mode == MARSHAL_JSON) 
                      ? seen->Marshal[EscObjInt64] 
                      : seen->Marshal[ObjInt64];

    /* Inject the named key string and the formatted numeric value digits string flawlessly */
    luaL_addstring(seen->B, lua_pushfstring(L, fmt, elm->key, lua_tostring(L, -1)));

    lua_pop(L, 2); // Balance the stack registers frame back to neutral
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
    if(seen->mode == MARSHAL_BASH && !seen->isRoot)
        luaL_addstring(seen->B, seen->Marshal[BashArr]);
    else
	    luaL_addstring(seen->B, seen->Marshal[OpenArr]);

	return 0;
}

static int marshal_array_close(ref *seen)
{
	// Replace strcat with Buffer Append
    if(seen->mode == MARSHAL_BASH && !seen->isRoot)
        luaL_addstring(seen->B, seen->Marshal[BashClose]);
    else
	    luaL_addstring(seen->B, seen->Marshal[CloseArr]);

	return 0;
}

static int marshal_array_string(lua_State *L, json_elm *elm, ref *seen)
{
	// 1. Get pointer to source string
	elm->val = luaL_checklstring(L, -1, &elm->vlen);

	// 2. Format and Append
	if (elm->val == NULL_CACHE) {

        if(seen->mode == MARSHAL_BASH) 
            elm->val = BASH_NULL;

		luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrNull], elm->val));

        seen->nulls++;
    }
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
    /* Duplicate the item to ensure stack safety boundaries */
    lua_pushvalue(L, -1); // Stack: [..., val, val_copy]

    double d = luaL_checknumber(L, -1);

    /* 1. SEPARATE INTEGERS FROM FLOATING-POINT DECIMALS LOCALLY IN C */
    if (d == (int64_t)d) {
        /* Format the 64-bit value into a string buffer inside our C layer first.
         * This protects the upper bits from being sliced down to 32 bits! */
        char scratch[64];
        int len = snprintf(scratch, sizeof(scratch), "%" PRId64, (int64_t)d);
        lua_pushlstring(L, scratch, len); // Stack: [..., val, val_copy, digit_string]
        
        /* Map through the string passthrough '%s' specifier array slot (ArrNull).
         * This drops raw unquoted numerical text characters straight into the output stream. */
        luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrNull], lua_tostring(L, -1)));
        lua_pop(L, 3); // Balance stack registers frame back to neutral
        return 0;
    }

    /* 2. DEFAULT FLOATING-POINT PATH (%f handles decimals natively) */
    luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrNumber], d));
    lua_pop(L, 2);

    return 0;
}

static int marshal_array_int64(lua_State *L, ref *seen)
{
    /* Duplicate the target value onto the top of the stack frame */
    lua_pushvalue(L, -1); // Stack: [..., udata, udata_copy]

    /* Safely unbox the raw 8-byte integer out of the memory block */
    json_int64 *ud = (json_int64 *)luaL_checkudata(L, -1, "JSON.int64");
    int64_t val = ud->value;

    /* Format to string locally to guarantee cross-version 64-bit safety */
    char scratch[32];
    int len = snprintf(scratch, sizeof(scratch), "%" PRId64, val);
    lua_pushlstring(L, scratch, len); // Stack: [..., udata, udata_copy, digit_string]

    /* Map through your active format specifiers array (ArrInteger) using %s!
     * Since ArrInteger is configured as "%s" or "%d" inside your configuration arrays, 
     * passing the string representation to a %s specifier injects the text perfectly. */
    luaL_addstring(seen->B, lua_pushfstring(L, seen->Marshal[ArrInt64], lua_tostring(L, -1)));

    /* Clean the stack registers back to a balanced state */
    lua_pop(L, 2); // Pop digit_string and the string pushed by lua_pushfstring

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
	m->obj_int64 = &marshal_object_int64;
	m->obj_bool = &marshal_object_bool;
	// array metods
	m->arr_open = &marshal_array_open;
	m->arr_close = &marshal_array_close;
	m->arr_string = &marshal_array_string;
	m->arr_number = &marshal_array_number;
	m->arr_int64 = &marshal_array_int64;
	m->arr_bool = &marshal_array_bool;

	return m;
};