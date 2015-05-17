#include <stdint.h>
#include "linkedlist.h"


/* ============== STRUCTURE DEFINITIONS =============== */
typedef enum {NONE = 0, AND, OR} fuzzy_op;
typedef enum {INPUT = 0, OUTPUT} ling_var_type;

/* holds the linguistic values, e.g. HIGH, LOW, MEDIUM etc */
typedef struct ling_val_struct {
    char* name;
    double a, b, c, d;
} ling_val;

typedef double (*triangular_mf)(ling_val*, double);

/* holds the linguistic_variables, e.g. TEMPERATURE, HEIGHT etc */
typedef struct ling_var_struct {
    char* name;
    double value;   
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
    double result;
} rule_consequent;

/* holds the antecedent and consequent of a rule*/
typedef struct rule_struct {
    rule_antecedent* antecedent;
    rule_consequent* consequent;
    double result;
} fuzzy_rule;

/* holds everything together */
typedef struct fuzzy_engine_struct {
    linked_list* ling_vars;
    linked_list* rules;
} fuzzy_engine;

/*
 * Allocates a fuzzy_egine structre that holds all the components of
 * the system
*/
fuzzy_engine* create_fuzzy_engine();
/*
 * Adds a linguistic variable to a fuzzy_engine
*/
void add_ling_var(fuzzy_engine*, ling_var*);
/*
 * Adds a rule to a fuzzy_engine
*/
void add_rule(fuzzy_engine*, fuzzy_rule*);

/*
 * Allocates memory for a lingustic variable and sets its name, id
 * and variable type (INPUT, OUTPUT)
*/
ling_var* create_linguistic_variable(const char* name, int id, ling_var_type);
/*
 * Adds a linguistic value to a linguistic variable
*/
void add_ling_val(ling_var*, ling_val*);

/*
 * Allocates memory for a linguistic value and sets the name
 * and triangular shape boundries for it
*/
ling_val* create_linguistic_value(const char* name,
                                  double a, double b, double c, double d);

/*
 * Allocates memory for a rule and sets its antecentent and consequent
*/
fuzzy_rule* create_rule(rule_antecedent*, rule_consequent*);

/*
 * Allocates memory for a rule anatecedent
*/
rule_antecedent* create_rule_antecedent();
/*
 * Adds a condition to a rule antecedent
*/
void add_condition_to_antecedent(rule_antecedent*, condition*);

/*
 * Allocates memory for a rule consequent and sets its linguistic
 * variable and its linguistc value
*/
rule_consequent* create_rule_consequent(ling_var*, ling_val*);

/*
 * Allocates memory for a condition and sets its variable, value and 
 * fuzzy operator that will be applied between the current condition
 * and the one that will follow it
 * Example:
 *  create_condition("temp", "high", AND)
 *  create_condition("humidity", "low", NONE)
 * translates to "if temp is high AND humidity is low"
*/
condition* create_condition(ling_var*, ling_val*, fuzzy_op);

/*
 * Outputs the egine's linguistic variables with their correspondig
 * linguistic variables and the rules
*/
void dump_engine(fuzzy_engine*);

/*
 * Sets an input value for a linguistic variable that is identified
 * by its name
*/
uint8_t register_value_by_name(fuzzy_engine* engine, char* name, double value);

/*
 * Sets an input value for a linguistic variable that is identified
 * by its id
*/
uint8_t register_value_by_id(fuzzy_engine* engine, int id, double value);

/*
 * Runs the whole fuzzy process
*/
 void run_fuzzy(fuzzy_engine* engine);

/* ============================ STATIC FUNCTIONS =========================== */
/*
 * Calculates the membership degree of a given input to a linguistic
 * value determined by a trapezium
*/
// double trapezium_mf(double input, ling_val* value);

/*
 * Goes through all the rules and calculates the resulting value
*/
// void evaluate_rules();

/*
 * Returns the maximum of two values
*/
// double max(double a, double b);

/*
 * Returns the minimum of two values
*/
// double min(double a, double b);

/*
 * Sets an input value for a linguistic variable that is identified
 * either by its id or its name
 * To be identified by its id, the name pointer must be 0
 * To be identified by its name, id must be less than 0
*/
// static uint8_t register_value(fuzzy_engine* engine, char*name, int id,
//                               double value)