#include <stdio.h>
#include "fuzzyengine.h"


/* ========================== FUZZY ENGINE LOGIC ========================== */
fuzzy_engine* create_fuzzy_engine()
{
    fuzzy_engine* engine = (fuzzy_engine*)malloc(sizeof(fuzzy_engine));
    engine->ling_vars = new_linked_list();
    engine->rules = new_linked_list();
    return engine;
}

void add_ling_var(fuzzy_engine* engine, ling_var* variable)
{
    engine->ling_vars->add(engine->ling_vars, variable);
}

void add_rule(fuzzy_engine* engine, fuzzy_rule* rule)
{
    engine->rules->add(engine->rules, rule);
}

/* ====================== LINGUISTIC VARIABLE LOGIC ======================= */
ling_var* create_linguistic_variable(const char* name, int id,
                                     ling_var_type type)
{
    uint8_t name_len = strlen(name);
    ling_var* var = (ling_var*)malloc(sizeof(ling_var));

    memset(var, 0, sizeof(ling_var));
    var->name = (char*)malloc(name_len + 1);
    strcpy(var->name, name);
    var->id = id;
    var->values = new_linked_list();
    var->type = type;

    return var;
}

void add_ling_val(ling_var* variable, ling_val* value)
{
    variable->values->add(variable->values, value);
}

/* ======================= LINGUISTIC VALUE LOGIC ========================= */
ling_val* create_linguistic_value(const char* name, double a, double b,
                                  double c, double d)
{
    uint8_t name_len = strlen(name);
    ling_val* value = (ling_val*)malloc(sizeof(ling_val));
    memset(value, 0, sizeof(ling_val));
    value->name = (char*)malloc(name_len + 1);
    strcpy(value->name, name);
    value->a = a;
    value->b = b;
    value->c = c;
    value->d = d;

    return value;
}

/* ======================== FUZZY RULE LOGIC ============================== */
fuzzy_rule* create_rule(rule_antecedent* antecedent, rule_consequent* consequent)
{
    fuzzy_rule* rule = (fuzzy_rule*)malloc(sizeof(fuzzy_rule));
    memset(rule, 0, sizeof(fuzzy_rule));
    rule->antecedent = antecedent;
    rule->consequent = consequent;

    return rule;
}
void set_rule_antecedent(fuzzy_rule* rule, rule_antecedent* antecedent)
{
    rule->antecedent = antecedent;
}

void set_rule_consequent(fuzzy_rule* rule, rule_consequent* consequent)
{
    rule->consequent = consequent;
}

/* ====================== ANTECEDENT AND CONSEQUENT LOGIC ================= */
condition* create_condition(ling_var* variable, ling_val* value, fuzzy_op op)
{
    condition* cond = (condition*)malloc(sizeof(condition));
    memset(cond, 0, sizeof(condition));
    cond->variable = variable;
    cond->value = value;
    cond->op = op;
    return cond;
}

rule_antecedent* create_rule_antecedent()
{
    rule_antecedent* antecedent = new_linked_list();
    return antecedent;
}

void add_condition_to_antecedent(rule_antecedent* antecedent, condition* cond)
{
    antecedent->add(antecedent, cond);
}

rule_consequent* create_rule_consequent(ling_var* variable, ling_val* value)
{
    rule_consequent* consequent = (rule_consequent*)new_linked_list();
    memset(consequent, 0, sizeof(rule_consequent));
    consequent->variable = variable;
    consequent->value = value;
    return consequent;
}

void dump_engine(fuzzy_engine* engine) {
    printf("Linguistic variables\n====================\n");
    linked_list_node* node = engine->ling_vars->head;
    linked_list_node* aux_node;
    ling_var* var;
    ling_val* val;
    while(node != 0) {
        var = (ling_var*)node->data;
        printf("%d %d %s\n", var->id, var->type, var->name);
        aux_node = var->values->head;
        printf("   ");
        while(aux_node != 0) {
            val = (ling_val*)aux_node->data;
            printf("%s ", val->name);
            aux_node = aux_node->next;
        }
        printf("\n");
        node = node->next;
    }

    printf("Rules\n=====\n");
    node = engine->rules->head;
    condition* cond;
    while(node != 0) {
        fuzzy_rule* rule = node->data;
        aux_node = rule->antecedent->head;
        printf("if ");
        while(aux_node != 0) {
            cond = aux_node->data;
            printf("%s is %s %d ", cond->variable->name, cond->value->name,
                   cond->op);
            aux_node = aux_node->next;
        }

        printf("then %s is %s\n", rule->consequent->variable->name,
               rule->consequent->value->name);
        node = node->next;
    }
}