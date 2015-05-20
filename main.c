#include <stdlib.h>
#include <stdio.h>
#include "fuzzyengine.h"


void init_fuzzy_engine(fuzzy_engine* engine) {
    // TEMPERATURE
    ling_var* temp_err = create_linguistic_variable("temp_err", 1, INPUT);
    ling_val* low_temp = create_linguistic_value("low", -100, -100, 1.75, 2.5);
    ling_val* moderate_temp = create_linguistic_value("moderate",
                                                      1.5, 2.75, 3.75, 5);
    ling_val* high_temp = create_linguistic_value("high", 3.5, 5.5, 100, 100);
    add_ling_val(temp_err, low_temp);
    add_ling_val(temp_err, moderate_temp);
    add_ling_val(temp_err, high_temp);

    // HUMIDITY -- INPUT
    ling_var* humidity = create_linguistic_variable("humidity", 2, INPUT);
    ling_val* low_hum = create_linguistic_value("low", -10, -10, 20, 40);
    ling_val* moderate_hum = create_linguistic_value("moderate", 25, 47, 47, 70);
    ling_val* high_hum = create_linguistic_value("high", 45, 70, 100, 100);
    add_ling_val(humidity, low_hum);
    add_ling_val(humidity, moderate_hum);
    add_ling_val(humidity, high_hum);

    // RATE OF COOLING -- INPUT
    ling_var* rate_of_cooling = create_linguistic_variable("roc", 3, INPUT);
    ling_val* high_roc = create_linguistic_value("high", 0, 0, 4.5, 9);
    ling_val* moderate_roc = create_linguistic_value("moderate",
                                                     5, 11, 12.5, 18);
    ling_val* low_roc = create_linguistic_value("low", 13, 20, 100, 100);
    add_ling_val(rate_of_cooling, low_roc);
    add_ling_val(rate_of_cooling, moderate_roc);
    add_ling_val(rate_of_cooling, high_roc);

    // RATE OF HEATING -- INPUT
    ling_var* rate_of_heating = create_linguistic_variable("roh", 3, INPUT);
    ling_val* high_roh = create_linguistic_value("high", 0, 0, 4.5, 9);
    ling_val* moderate_roh = create_linguistic_value("moderate",
                                                     7, 12.5, 12.5, 18);
    ling_val* low_roh = create_linguistic_value("low", 15, 25, 100, 100);
    add_ling_val(rate_of_heating, low_roh);
    add_ling_val(rate_of_heating, moderate_roh);
    add_ling_val(rate_of_heating, high_roh);

    // HEATING STATUS -- OUTPUT
    ling_var* heat_status = create_linguistic_variable("heat_status", 3,
                                                       OUTPUT);
    ling_val* heat_on = create_linguistic_value("on", 0, 0, 30, 65);
    ling_val* heat_off = create_linguistic_value("off", 35, 50, 100, 100);
    add_ling_val(heat_status, heat_on);
    add_ling_val(heat_status, heat_off);

    add_ling_var(engine, temp_err);
    add_ling_var(engine, humidity);
    add_ling_var(engine, rate_of_cooling);
    add_ling_var(engine, rate_of_heating);
    add_ling_var(engine, heat_status);

    // ============================== RULES ==================================

    // if temp_err is high and rate_of_heating is high heat_status is OFF
    rule_antecedent* antecedent_4 = create_rule_antecedent();
    condition* cond_41 = create_condition(temp_err, high_temp, AND);
    condition* cond_42 = create_condition(humidity, high_hum, NONE);
    add_condition_to_antecedent(antecedent_4, cond_41);
    add_condition_to_antecedent(antecedent_4, cond_42);
    rule_consequent* consequent_4 = create_rule_consequent(heat_status,
                                                           heat_off);
    fuzzy_rule* rule_4 = create_rule(engine, antecedent_4, consequent_4);
    add_rule(engine, rule_4);

    // if temp_err is higg and rate_of_heating is low then heat_status is ON
    rule_antecedent* antecedent_1 = create_rule_antecedent();
    condition* cond_11 = create_condition(temp_err, high_temp, AND);
    condition* cond_12 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent_1, cond_11);
    add_condition_to_antecedent(antecedent_1, cond_12);
    rule_consequent* consequent_1 = create_rule_consequent(heat_status,
                                                           heat_on);
    fuzzy_rule* rule_1 = create_rule(engine, antecedent_1, consequent_1);
    add_rule(engine, rule_1);

    // if temp_err is low and rate_of_heating is low heat_status is ON
    rule_antecedent* antecedent_2 = create_rule_antecedent();
    condition* cond_21 = create_condition(temp_err, low_temp, AND);
    condition* cond_22 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent_2, cond_21);
    add_condition_to_antecedent(antecedent_2, cond_22);
    rule_consequent* consequent_2 = create_rule_consequent(heat_status,
                                                           heat_on);
    fuzzy_rule* rule_2 = create_rule(engine, antecedent_2, consequent_2);
    add_rule(engine, rule_2);

    // if temp_err is low and rate_of_heating is high heat_status is OFF
    rule_antecedent* antecedent_3 = create_rule_antecedent();
    condition* cond_31 = create_condition(temp_err, low_temp, AND);
    condition* cond_32 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent_3, cond_31);
    add_condition_to_antecedent(antecedent_3, cond_32);
    rule_consequent* consequent_3 = create_rule_consequent(heat_status,
                                                           heat_off);
    fuzzy_rule* rule_3 = create_rule(engine, antecedent_3, consequent_3);
    add_rule(engine, rule_3);

    register_value_by_name(engine, "temp_err", 5);
    register_value_by_name(engine, "roh", 20);
    register_value_by_name(engine, "humidity", 80);

    run_fuzzy(engine);

}

int main() {
    fuzzy_engine* engine = create_fuzzy_engine();
    init_fuzzy_engine(engine);
    dump_engine(engine);
    return 0;
}