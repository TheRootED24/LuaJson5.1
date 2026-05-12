#!/bin/bash

echo "=== 1. Invoking C/Lua Engine for Nested Payload ==="
# Capture the output from your script. 
# Make sure your test-bash command matches your CLI argument layout.
parent_obj=$(./test-bash -o "test" "me" "age" 99 "root" false "arr" "🍕 Food" "🎵 Music" "💻 Tech")

echo "Generated Bash Payload:"
echo "$parent_obj"
echo "--------------------------------------------------------"

echo -e "\n=== 2. Parsing the Parent Object ==="
# Declare an associative array (dictionary) for the parent object
declare -A my_obj

# Evaluate the payload directly into native Bash memory
eval "my_obj=$parent_obj"

# Print an item from the main Object
echo "Successfully extracted from Object -> [test]: ${my_obj[test]}"
echo "Successfully extracted from Object -> [age]:  ${my_obj[age]}"

echo -e "\n=== 3. Unpacking and Parsing the Nested Array ==="
# Extract the nested string payload out of the parent object's "arr" key
nested_arr_str="${my_obj[arr]}"
echo "Extracted Nested String: $nested_arr_str"

# Declare a standard indexed array for the child list
declare -a child_arr

# Evaluate the inner string structure into the new indexed array
eval "child_arr=$nested_arr_str"

# Print an item from the nested Array
echo "Successfully extracted from Child Array -> Index 0: ${child_arr[0]}"
echo "Successfully extracted from Child Array -> Index 1: ${child_arr[1]}"
