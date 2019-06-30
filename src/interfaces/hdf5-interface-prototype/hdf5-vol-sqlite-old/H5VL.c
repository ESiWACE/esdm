#include <stdio.h>

#include <stdlib.h>

typedef enum type_t { TYPE_SCHUHE,
  TYPE_BANANE } type_t;


struct box_t {
  double x;
  double y;
  double z;
};


struct pig_t {
  int days_old;
  int weight;
};


typedef struct H5VL_loc_params_t {
  type_t type;
  union {
    struct box_t box_t;
    struct pig_t pig_t;
  } loc_data;
} H5VL_loc_params_t;


void do_something(void *obj, H5VL_loc_params_t etype) {
  switch (etype.type) {
    case TYPE_SCHUHE: {
      (etype.loc_data.box_t) *box = ((etype.loc_data.box_t) *)obj;
      printf("%f\n", box->x);

    } break;
    case TYPE_BANANE: {
      (etype.loc_data.pig_t) *pig = ((etype.loc_data.pig_t) *)obj;
      printf("%d\n", pig->days_old);

    } break;
    default:
      fprintf(stderr, "unknown type");

      exit(1);
  }
}

int main(int argc, char **argv) {
  box_t box;

  //box.x = 35.5;

  //box.y = 20.0;

  //box.z = 15.1;


  pig_t pig;

  //pig.days_old = 230;

  //pig.weight = 85;

  H5VL_loc_params_t par;
  par.type             = TYPE_SCHUHE;
  par.loc_data.box_t.x = 35.5;

  H5VL_loc_params_t parr;
  parr.type                    = TYPE_BANANE;
  parr.loc_data.pig_t.days_old = 150;


  //par.type = TYPE_SCHUHE;

  do_something(&box, &par);

  do_something(&pig, &parr);

  return 0;
}
