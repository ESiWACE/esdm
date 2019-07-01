#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <smd.h>

int main(){
	//Test  smd_attr_new
	printf("Test smd_attr_new:\n");
	int32_t value = 10;
	smd_attr_t * attr = smd_attr_new("test1", SMD_DTYPE_INT32, & value, 0);

	char* test = "string";
	smd_attr_t * attr1 = smd_attr_new("teststring",SMD_DTYPE_STRING, test,1);
	char * attr1_value = (char *)smd_attr_get_value(attr1);
	assert(strcmp(attr1_value,test)==0);

	//test array
	int32_t  value2[3] = {1, 2, 3};
	smd_dtype_t * array_type = smd_type_array(SMD_DTYPE_INT32,3);
	smd_attr_t * attr2 = smd_attr_new("test2", array_type, value2, 2);
	int32_t * attr2_value = (int32_t*)smd_attr_get_value(attr2);  
	for(int i = 0;i < 3;i++){
		assert(value2[i] == attr2_value[i]); 
	}
	//test 2darray 
	int32_t value2d [2][3] = {{1,2,3},{4,5,6}};
	smd_dtype_t * array2d_type = smd_type_array(array_type,2);
	smd_attr_t * attr2d = smd_attr_new("test2d",array2d_type,value2d,200);
	int32_t * attr2d_value = (int32_t *)smd_attr_get_value(attr2d) ;  
	for(int i = 0;i < 6;i++){
		assert((*value2d)[i] == attr2d_value[i]); 
	}
	printf("Pass\n");

	//test get type
	printf("Test get type:\n");
	assert(smd_attr_get_type(attr) == SMD_TYPE_INT32); 
	assert(smd_attr_get_type(attr1) == SMD_TYPE_STRING); 
	assert(smd_attr_get_type(attr2) == SMD_TYPE_ARRAY); 

	printf("Pass\n");

	//test copy_value
	printf("Test smd_attr_copy_value:\n");
	int32_t value_copy;
	smd_attr_copy_value(attr, &value_copy);
	assert(value == value_copy);


	//for string
	char * value_copy1;
	smd_attr_copy_value(attr1, & value_copy1);
	assert(strcmp((char *)value_copy1,test)==0);


	//for array	
	int32_t * value_copy2 = (int32_t *)malloc(sizeof(int32_t) * 3);
	smd_attr_copy_value(attr2,value_copy2);
	for(int i = 0;i < 3;i++){
		assert(value2[i] == attr2_value[i]); 
	}
	free(value_copy2);
	printf("Pass\n");
	

	//test get name
	printf("Test smd_attr_get_name:\n");
	char value3 = 'X';
	smd_attr_t * attr3 = smd_attr_new("test3", SMD_DTYPE_CHAR, & value3,3);
	char * value4 = "Test 4";
	smd_attr_t * attr4 = smd_attr_new("test3", SMD_DTYPE_STRING, value4, 4);
	smd_attr_t * attr5 = smd_attr_new("test5", SMD_DTYPE_EMPTY, NULL, 5);
	
	assert(strcmp(smd_attr_get_name(attr), "test1") == 0);
	assert(strcmp(smd_attr_get_name(attr3), "test3") == 0);
	assert(strcmp(smd_attr_get_name(attr4), "test3") == 0);
	printf("Pass\n");



	//test linking
	printf("Test smd_attr_link:\n");
	int attr_count = smd_attr_count(attr);
	assert(attr_count == 0);
	smd_link_ret_t link_ret = smd_attr_link(attr, attr3, 0);
	assert(link_ret == SMD_ATTR_LINKED);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 1);

	link_ret = smd_attr_link(attr, attr4, 0);
	assert(link_ret == SMD_ATTR_EEXIST);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 1);

	link_ret = smd_attr_link(attr, attr4, 1);
	assert(link_ret == SMD_ATTR_REPLACED);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 1);

	link_ret = smd_attr_link(attr, attr5, 0);
	assert(link_ret == SMD_ATTR_LINKED);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 2);
	


	//Relinking existing attributed will free the attribute but still link.
	//link_ret = smd_attr_link(attr,attr5,1);
	//assert(link_ret == SMD_ATTR_REPLACED);

	printf("Pass\n");

	
	//Test find pos
	//Check find position and is attr3 is replaced
	printf("Test smd_find_position_by_id:\n");
	int pos_ret = smd_find_position_by_id(attr, 4);
	assert(pos_ret == 0);

	pos_ret = smd_find_position_by_id(attr, 5);
	assert(pos_ret == 1);

	pos_ret = smd_find_position_by_id(attr, 3);
	assert(pos_ret == -1);
	printf("Pass\n");

	//repeat find position with name search
	printf("Test smd_find_position_by_name:\n");
	pos_ret = smd_find_position_by_name(attr, "test3");
	assert(pos_ret == 0);

	pos_ret = smd_find_position_by_name(attr, "test5");
	assert(pos_ret == 1);

	pos_ret = smd_find_position_by_name(attr, "test1");
	assert(pos_ret == -1);

	printf("Pass\n");


	//test find get child by name
	printf("Test smd_attr_get_child_by_name:\n");
	smd_attr_t * attr_ret = smd_attr_get_child_by_name(attr, "test3");
	assert(attr_ret == attr4);

	attr_ret = smd_attr_get_child_by_name(attr, "test5");
	assert(attr_ret == attr5);

	attr_ret = smd_attr_get_child_by_name(attr, "test1");
	assert(attr_ret == NULL);

	printf("Pass\n");


	//test unlinking
	printf("Test smd_attr_unlink_pos:\n");
	smd_attr_unlink_pos(attr, 0);
	pos_ret = smd_find_position_by_id(attr, 4);
	assert(pos_ret == -1);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 1);
	pos_ret = smd_find_position_by_id(attr, 5);
	assert(pos_ret == 0);

	smd_attr_unlink_pos(attr, 0);
	pos_ret = smd_find_position_by_id(attr, 5);
	assert(pos_ret == -1);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 0);


	//test unlinking in reverse order
	link_ret = smd_attr_link(attr, attr4, 0);
	link_ret = smd_attr_link(attr, attr5, 0);
	assert(smd_find_position_by_id(attr, 4) == 0);
	assert(smd_find_position_by_id(attr, 5) == 1);

	smd_attr_unlink_pos(attr,1);
	pos_ret = smd_find_position_by_id(attr ,5);
	assert(pos_ret == -1);
	pos_ret = smd_find_position_by_id(attr, 4);
	assert(pos_ret == 0);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 1);
	smd_attr_unlink_pos(attr,0);
	pos_ret = smd_find_position_by_id(attr, 4);
	assert(pos_ret == -1);
	attr_count = smd_attr_count(attr);
	assert(attr_count == 0);
	printf("Pass\n");

	//test get child
	printf("Test smd_attr_get_child:\n");
	link_ret = smd_attr_link(attr, attr4, 0);
	link_ret = smd_attr_link(attr, attr5, 0);
	smd_attr_t * child_attr = smd_attr_get_child(attr, 0);
	assert(child_attr == attr4);
	child_attr = smd_attr_get_child(attr, 1);
	assert(child_attr == attr5);


	printf("Pass\n");

	//test attr count
	int count = smd_attr_count(attr);
	assert(count == 2);

	//iter tested in basic_types


	return 0;
}
