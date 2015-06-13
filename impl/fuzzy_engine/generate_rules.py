rules = [{'temp_err' : 'low', 'humidity': 'low', 'rate_of_heating': 'low' , 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'low', 'rate_of_heating': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'low', 'rate_of_heating': 'high', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'moderate', 'rate_of_heating': 'low', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'moderate', 'rate_of_heating': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'moderate', 'rate_of_heating': 'high', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'high', 'rate_of_heating': 'low', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'high', 'rate_of_heating': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'high', 'rate_of_heating': 'high', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'low', 'rate_of_heating': 'low', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'low', 'rate_of_heating': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'low', 'rate_of_heating': 'high', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'moderate', 'rate_of_heating': 'low', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'moderate', 'rate_of_heating': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'moderate', 'rate_of_heating': 'high', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'high', 'rate_of_heating': 'low', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'high', 'rate_of_heating': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'high', 'rate_of_heating': 'high', 'heat_status':'off'},
         {'temp_err' : 'high', 'humidity': 'low', 'rate_of_heating': 'low', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'low', 'rate_of_heating': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'low', 'rate_of_heating': 'high', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'moderate', 'rate_of_heating': 'low', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'moderate', 'rate_of_heating': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'moderate', 'rate_of_heating': 'high', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'high', 'rate_of_heating': 'low', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'high', 'rate_of_heating': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'high', 'rate_of_heating': 'high', 'heat_status':'on'},
         # rate of cooling
         {'temp_err' : 'low', 'humidity': 'low', 'rate_of_cooling': 'low' , 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'low', 'rate_of_cooling': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'low', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'low', 'humidity': 'moderate', 'rate_of_cooling': 'low', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'moderate', 'rate_of_cooling': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'moderate', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'low', 'humidity': 'high', 'rate_of_cooling': 'low', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'high', 'rate_of_cooling': 'moderate', 'heat_status':'off'},
         {'temp_err' : 'low', 'humidity': 'high', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'low', 'rate_of_cooling': 'low', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'low', 'rate_of_cooling': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'low', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'moderate', 'rate_of_cooling': 'low', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'moderate', 'rate_of_cooling': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'moderate', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'high', 'rate_of_cooling': 'low', 'heat_status':'off'},
         {'temp_err' : 'moderate', 'humidity': 'high', 'rate_of_cooling': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'moderate', 'humidity': 'high', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'low', 'rate_of_cooling': 'low', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'low', 'rate_of_cooling': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'low', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'moderate', 'rate_of_cooling': 'low', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'moderate', 'rate_of_cooling': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'moderate', 'rate_of_cooling': 'high', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'high', 'rate_of_cooling': 'low', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'high', 'rate_of_cooling': 'moderate', 'heat_status':'on'},
         {'temp_err' : 'high', 'humidity': 'high', 'rate_of_cooling': 'high', 'heat_status':'on'}]

var_names = {
 "temp_err": {"high":"high_temp", "moderate": "moderate_temp", "low": "low_temp"},
 "humidity": {"high": "high_hum", "moderate": "moderate_hum", "low": "low_hum"},
 "rate_of_heating": {"high": "high_roh", "moderate": "moderate_roh", "low": "low_roh"},
 "rate_of_cooling": {"high": "high_roc", "moderate": "moderate_roc", "low": "low_roc"},
 "heat_status": {"on": "heat_on", "off": "heat_off"}
}


rule_count = 1
with open("rules.cc", "w") as file_:
    file_.write("rule_antecedent* antecedent;\n")
    file_.write("condition* cond1;\n")
    file_.write("condition* cond2;\n")
    file_.write("condition* cond3;\n")
    file_.write("rule_consequent* consequent;\n")
    file_.write("fuzzy_rule* rule;\n\n")

    for rule in rules:

        file_.write(
            "antecedent = create_rule_antecedent();\n".format(rule_count))
        file_.write(
            "cond1 = create_condition(temp_err, {}, AND);\n".format(
                var_names["temp_err"][rule["temp_err"]]))

        file_.write(
            "cond2 = create_condition(humidity, {}, AND);\n".format(
                var_names["humidity"][rule["humidity"]]))

        if "rate_of_heating" in rule:
            file_.write(
                "cond3 = create_condition(rate_of_heating, {}, NONE);\n".format(
                    var_names["rate_of_heating"][rule["rate_of_heating"]]))
        else:
            file_.write(
                "cond3 = create_condition(rate_of_cooling, {}, NONE);\n".format(
                    var_names["rate_of_cooling"][rule["rate_of_cooling"]]))

        file_.write("add_condition_to_antecedent(antecedent, cond1);\n")
        file_.write("add_condition_to_antecedent(antecedent, cond2);\n")
        file_.write("add_condition_to_antecedent(antecedent, cond3);\n")

        file_.write(
            "consequent = create_rule_consequent(engine, heat_status, {});\n".format(
                var_names["heat_status"][rule["heat_status"]]))

        file_.write(
            "rule = create_rule(antecedent, consequent);\n")

        file_.write("add_rule(engine, rule);\n\n".format(rule_count))

        rule_count += 1







