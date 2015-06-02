#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "linkedlist_comp.h"

#define POINTS_ARE_EQUAL(p1, p2) (p1.x == p2.x && p1.y == p2.y)

/* ============== STRUCTURE DEFINITIONS =============== */
typedef enum {NONE = 0, AND, OR} fuzzy_op;
typedef enum {INPUT = 0, OUTPUT} ling_var_type;

/* holds the linguistic values, e.g. HIGH, LOW, MEDIUM etc */
typedef struct ling_val_struct {
    char* name;
    float a, b, c, d;
} ling_val;

typedef float (*triangular_mf)(ling_val*, float);

/* holds the linguistic_variables, e.g. TEMPERATURE, HEIGHT etc */
typedef struct ling_var_struct {
    char* name;
    float value;
    int id;
    linked_list* values;
    ling_var_type type;
} ling_var;

/* holds part of an atecetend, e.g. TEMPERATURE is HIGH */
typedef struct condition_struct {
    ling_var* variable;
    ling_val* value;
    fuzzy_op op;
} condition;

/* holds the atecedent of a rule,
   e.g. TEMPERATURE is HIGH AND HUMIDITY is LOW*/
typedef linked_list rule_antecedent;
typedef struct rule_consequent_struct {
    ling_var* variable;
    ling_val* value;
    float result;
} rule_consequent;

/* holds the antecedent and consequent of a rule*/
typedef struct rule_struct {
    rule_antecedent* antecedent;
    rule_consequent* consequent;
    float result;
} fuzzy_rule;

/* holds everything together */
typedef struct fuzzy_engine_struct {
    linked_list* ling_vars;
    linked_list* consequents;
    linked_list* rules;
} fuzzy_engine;

typedef struct point_struct {
    float x, y;
} point;

/*
 * Allocates a fuzzy_egine structre that holds all the components of
 * the system
*/
fuzzy_engine*
create_fuzzy_engine();
/*
 * Adds a linguistic variable to a fuzzy_engine
*/
void
add_ling_var(fuzzy_engine*, ling_var*);
/*
 * Adds a rule to a fuzzy_engine
*/
void
add_rule(fuzzy_engine*, fuzzy_rule*);

/*
 * Allocates memory for a lingustic variable and sets its name, id
 * and variable type (INPUT, OUTPUT)
*/
ling_var*
create_linguistic_variable(const char* name, int id, ling_var_type);
/*
 * Adds a linguistic value to a linguistic variable
*/
void
add_ling_val(ling_var*, ling_val*);

/*
 * Allocates memory for a linguistic value and sets the name
 * and triangular shape boundries for it
*/
ling_val*
create_linguistic_value(const char* name,
                                  float a, float b, float c, float d);

/*
 * Allocates memory for a rule and sets its antecentent and consequent
*/
fuzzy_rule*
create_rule(fuzzy_engine*, rule_antecedent*, rule_consequent*);

/*
 * Allocates memory for a rule anatecedent
*/
rule_antecedent*
create_rule_antecedent();
/*
 * Adds a condition to a rule antecedent
*/
void
add_condition_to_antecedent(rule_antecedent*, condition*);

/*
 * Allocates memory for a rule consequent and sets its linguistic
 * variable and its linguistc value
*/
rule_consequent*
create_rule_consequent(ling_var*, ling_val*);

/*
 * Allocates memory for a condition and sets its variable, value and
 * fuzzy operator that will be applied between the current condition
 * and the one that will follow it
 * Example:
 *  create_condition("temp", "high", AND)
 *  create_condition("humidity", "low", NONE)
 * translates to "if temp is high AND humidity is low"
*/
condition*
create_condition(ling_var*, ling_val*, fuzzy_op);

/*
 * Outputs the egine's linguistic variables with their correspondig
 * linguistic variables and the rules
*/
void
dump_engine(fuzzy_engine*);

/*
 * Sets an input value for a linguistic variable that is identified
 * by its name
*/
uint8_t
register_value_by_name(fuzzy_engine* engine, char* name, float value);

/*
 * Sets an input value for a linguistic variable that is identified
 * by its id
*/
uint8_t
register_value_by_id(fuzzy_engine* engine, int id, float value);

/*
 * Runs the whole fuzzy process
*/
 point*
 run_fuzzy(fuzzy_engine* engine);

/* ============================ STATIC FUNCTIONS =========================== */
/*
 * Calculates the membership degree of a given input to a linguistic
 * value determined by a trapezium
*/
// float trapezium_mf(float input, ling_val* value);

/*
 * Goes through all the rules and calculates the resulting value
*/
// void evaluate_rules();

/*
 * Returns the maximum of two values
*/
// float max(float a, float b);

/*
 * Returns the minimum of two values
*/
// float min(float a, float b);

/*
 * Sets an input value for a linguistic variable that is identified
 * either by its id or its name
 * To be identified by its id, the name pointer must be 0
 * To be identified by its name, id must be less than 0
*/
// uint8_t register_value(fuzzy_engine* engine, char*name, int id,
//                               float value)

/*
 * Defuzzyfies the rules' outputs using the centroid method
*/
// point* defuzzify(fuzzy_engine* engine)

/*
 * Insert sort that orders consequents by their values' a attribute
*/
//void order_consequents_geometrically(rule_consequent** consequent_list,
//                                     int length)
