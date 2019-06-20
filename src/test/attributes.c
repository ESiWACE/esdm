#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include <smd.h>

static int count;
static void iter(int id, const char * name){
	printf("%d %s\n",id,name);
	count++;
}


int main(){
	assert(smd_type_get_size(SMD_DTYPE_INT32) == sizeof(int32_t));
	int32_t id = 1;
	int32_t value = 10;

	smd_attr_t * attr = smd_attr_new("test1", SMD_DTYPE_INT32, & value, id);


	char* test = "string";
	smd_attr_t * attrs = smd_attr_new("tests",SMD_DTYPE_STRING, test,id);
	char * atv = (char *)smd_attr_get_value(attrs);
	printf("Atv: %s\n", atv);

	int32_t value_copy;
	smd_attr_copy_value(attr, &value_copy);
	assert(value == value_copy);

	int32_t  value2[3] = {1, 2, 3};
	printf("o: %lld %lld %lld\n",&value2[0],&value2[1],&value2[2]);
	smd_dtype_t * array_type = smd_type_array(SMD_DTYPE_INT32,3);
	smd_attr_t * attr2 = smd_attr_new("test2", array_type, value2, 2);
	
	int32_t * attr2_value = (int32_t*)smd_attr_get_value(attr2);  
	printf("%p,%p\n",attr2_value,value2);
	for(int i = 0;i < 3;i++){
		assert(value2[i] == attr2_value[i]); 
	}
	
	//int32_t * value_copy2 = (int32_t *)malloc(sizeof(int32_t) * 3);
	//smd_attr_copy_value(attr2,value_copy2);
	//printf("%d\n",value_copy2[1]);
	//free(value_copy2);
	

	char value3 = 'X';
	smd_attr_t * attr3 = smd_attr_new("test3", SMD_DTYPE_CHAR, & value3,3);
	char * value4 = "Test 4";
	smd_attr_t * attr4 = smd_attr_new("test3", SMD_DTYPE_STRING, value4, 4);
	smd_attr_t * attr5 = smd_attr_new("test5", SMD_DTYPE_EMPTY, NULL, 5);
	

	//test get name
	assert(strcmp(smd_attr_get_name(attr), "test1") == 0);
	assert(strcmp(smd_attr_get_name(attr3), "test3") == 0);
	assert(strcmp(smd_attr_get_name(attr4), "test3") == 0);


	//test linking
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


	//Check find position and is attr3 is replaced
	int pos_ret = smd_find_position_by_id(attr, 4);
	assert(pos_ret == 0);

	pos_ret = smd_find_position_by_id(attr, 5);
	assert(pos_ret == 1);

	pos_ret = smd_find_position_by_id(attr, 3);
	assert(pos_ret == -1);

	//repeat find position with name search
	pos_ret = smd_find_position_by_name(attr, "test3");
	assert(pos_ret == 0);

	pos_ret = smd_find_position_by_name(attr, "test5");
	assert(pos_ret == 1);

	pos_ret = smd_find_position_by_name(attr, "test1");
	assert(pos_ret == -1);


	//test unlinking
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

	//test get child
	link_ret = smd_attr_link(attr, attr4, 0);
	link_ret = smd_attr_link(attr, attr5, 0);
	smd_attr_t * child_attr = smd_attr_get_child(attr, 0);
	assert(child_attr == attr4);
	child_attr = smd_attr_get_child(attr, 1);
	assert(child_attr == attr5);



	count = 0;
	//smd_iterate(attr,iter);
	//assert(count==1);
	printf("%d\n",count);

	return 0;
}
