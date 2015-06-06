#include <stdlib.h>
#include <stdio.h>
#include "fuzzyengine.h"


void init_fuzzy_engine(fuzzy_engine* engine) {
    // TEMPERATURE
    ling_var* temp_err = create_linguistic_variable("temp_err", 1, INPUT);
    ling_val* low_temp = create_linguistic_value("low", -100, -100, 1.75, 2.75);
    ling_val* moderate_temp = create_linguistic_value("moderate",
                                                      1.5, 2.75, 3.75, 5);
    ling_val* high_temp = create_linguistic_value("high", 3.75, 5.5, 100, 100);
    add_ling_val(temp_err, low_temp);
    add_ling_val(temp_err, moderate_temp);
    add_ling_val(temp_err, high_temp);

    // HUMIDITY -- INPUT
    ling_var* humidity = create_linguistic_variable("humidity", 2, INPUT);
    ling_val* low_hum = create_linguistic_value("low", -10, -10, 20, 40);
    ling_val* moderate_hum = create_linguistic_value("moderate", 20, 47, 47, 70);
    ling_val* high_hum = create_linguistic_value("high", 50, 70, 100, 100);
    add_ling_val(humidity, low_hum);
    add_ling_val(humidity, moderate_hum);
    add_ling_val(humidity, high_hum);

    // RATE OF COOLING -- INPUT
    ling_var* rate_of_cooling = create_linguistic_variable("roc", 3, INPUT);
    ling_val* high_roc = create_linguistic_value("low", 0, 0, -0.11, -0.28);
    ling_val* moderate_roc = create_linguistic_value("moderate",
                                                     -0.11, -0.32, -0.32, -0.5);
    ling_val* low_roc = create_linguistic_value("high", -0.35,- 0.5, -2, -2);
    add_ling_val(rate_of_cooling, low_roc);
    add_ling_val(rate_of_cooling, moderate_roc);
    add_ling_val(rate_of_cooling, high_roc);

    // RATE OF HEATING -- INPUT
    ling_var* rate_of_heating = create_linguistic_variable("roh", 4, INPUT);
    ling_val* high_roh = create_linguistic_value("low", 0, 0, 0.11, 0.28);
    ling_val* moderate_roh = create_linguistic_value("moderate",
                                                     0.11, 0.32, 0.32, 0.5);
    ling_val* low_roh = create_linguistic_value("high", 0.35, 0.5, 2, 2);
    add_ling_val(rate_of_heating, low_roh);
    add_ling_val(rate_of_heating, moderate_roh);
    add_ling_val(rate_of_heating, high_roh);

    // HEATING STATUS -- OUTPUT
    ling_var* heat_status = create_linguistic_variable("heat_status", 5,
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

    rule_antecedent* antecedent;
    condition* cond1;
    condition* cond2;
    condition* cond3;
    rule_consequent* consequent;
    fuzzy_rule* rule;

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, low_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, moderate_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_off);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, low_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, moderate_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, low_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, moderate_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);

    antecedent = create_rule_antecedent();
    cond1 = create_condition(temp_err, high_temp, AND);
    cond2 = create_condition(humidity, high_hum, AND);
    cond3 = create_condition(rate_of_heating, high_roh, NONE);
    add_condition_to_antecedent(antecedent, cond1);
    add_condition_to_antecedent(antecedent, cond2);
    add_condition_to_antecedent(antecedent, cond3);
    consequent = create_rule_consequent(engine, heat_status, heat_on);
    rule = create_rule(antecedent, consequent);
    add_rule(engine, rule);


    register_value_by_name(engine, "temp_err", 12);
    register_value_by_name(engine, "roh", 0.1);
    register_value_by_name(engine, "humidity", 90);

    point * p = run_fuzzy(engine);
    if(p == 0) {
        printf("bat shit\n");
        return;
    }
    printf("\nresult: x: %.4f  y: %.4f\n", p->x, p->y);

}

int main() {
    fuzzy_engine* engine = create_fuzzy_engine();
    init_fuzzy_engine(engine);
    return 0;
}
