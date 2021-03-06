#include <stdio.h>
#include "fuzzyengine.h"


/* ========================== FUZZY ENGINE LOGIC ========================== */
fuzzy_engine*
create_fuzzy_engine()
{
    fuzzy_engine* engine = (fuzzy_engine*)os_malloc(sizeof(fuzzy_engine));
    engine->ling_vars = new_linked_list();
    engine->rules = new_linked_list();
    engine->consequents = new_linked_list();
    return engine;
}

void
add_ling_var(fuzzy_engine* engine, ling_var* variable)
{
    engine->ling_vars->add(engine->ling_vars, variable);
}

void
add_rule(fuzzy_engine* engine, fuzzy_rule* rule)
{
    engine->rules->add(engine->rules, rule);
}

/* ====================== LINGUISTIC VARIABLE LOGIC ======================= */
ling_var*
create_linguistic_variable(const char* name, int id,
                                     ling_var_type type)
{
    uint8_t name_len = strlen(name);
    ling_var* var = (ling_var*)os_malloc(sizeof(ling_var));

    os_memset(var, 0, sizeof(ling_var));
    var->name = (char*)os_malloc(name_len + 1);
    strcpy(var->name, name);
    var->id = id;
    var->values = new_linked_list();
    var->type = type;

    return var;
}

void
add_ling_val(ling_var* variable, ling_val* value)
{
    variable->values->add(variable->values, value);
}

/* ======================= LINGUISTIC VALUE LOGIC ========================= */
ling_val*
create_linguistic_value(const char* name, double a, double b,
                                  double c, double d)
{
    uint8_t name_len = strlen(name);
    ling_val* value = (ling_val*)os_malloc(sizeof(ling_val));
    os_memset(value, 0, sizeof(ling_val));
    value->name = (char*)os_malloc(name_len + 1);
    strcpy(value->name, name);
    value->a = a;
    value->b = b;
    value->c = c;
    value->d = d;

    return value;
}

/* ======================== FUZZY RULE LOGIC ============================== */
fuzzy_rule*
create_rule(rule_antecedent* antecedent,
                        rule_consequent* consequent)
{
    fuzzy_rule* rule = (fuzzy_rule*)os_malloc(sizeof(fuzzy_rule));
    os_memset(rule, 0, sizeof(fuzzy_rule));
    rule->antecedent = antecedent;
    rule->consequent = consequent;

    return rule;
}
void
set_rule_antecedent(fuzzy_rule* rule, rule_antecedent* antecedent)
{
    rule->antecedent = antecedent;
}

void
set_rule_consequent(fuzzy_rule* rule, rule_consequent* consequent)
{
    rule->consequent = consequent;
}

/* ====================== ANTECEDENT AND CONSEQUENT LOGIC ================= */
condition*
create_condition(ling_var* variable, ling_val* value, fuzzy_op op)
{
    condition* cond = (condition*)os_malloc(sizeof(condition));
    os_memset(cond, 0, sizeof(condition));
    cond->variable = variable;
    cond->value = value;
    cond->op = op;
    return cond;
}

rule_antecedent*
create_rule_antecedent()
{
    rule_antecedent* antecedent = new_linked_list();
    return antecedent;
}

void
add_condition_to_antecedent(rule_antecedent* antecedent, condition* cond)
{
    antecedent->add(antecedent, cond);
}

/*
 * Creates a rule consequent if and only if it does not find
 * an identical one. If it does find an identical one, it will return it
 * Consequents will be singletones
*/
rule_consequent*
create_rule_consequent(fuzzy_engine* engine, ling_var* variable,
                       ling_val* value)
{
    linked_list_node* node = engine->consequents->head;
    while(node != 0) {
        rule_consequent* tmp_consequent = (rule_consequent*)node->data;
        if(tmp_consequent->variable == variable && tmp_consequent->value == value) {
            return tmp_consequent;
        }
        node = node->next;
    }

    rule_consequent* consequent = (rule_consequent*)new_linked_list();
    memset(consequent, 0, sizeof(rule_consequent));
    consequent->variable = variable;
    consequent->value = value;
    // add the new consequent to the list
    engine->consequents->add(engine->consequents, consequent);

    return consequent;
}

void dump_engine(fuzzy_engine* engine)
{
    char buffer[500];
    char buffer_tmp[600];
    serial_debug("Linguistic variables\n====================\n", DEBUG_1);
    linked_list_node* node = engine->ling_vars->head;
    linked_list_node* aux_node;
    ling_var* var;
    ling_val* val;
    while(node != 0) {
        var = (ling_var*)node->data;
        os_sprintf(buffer, "%d %d %s ", var->id, var->type, var->name);
        ftoa(var->value, buffer);
        serial_debug(buffer, DEBUG_1);
        aux_node = var->values->head;
        serial_debug("   ", DEBUG_1);
        while(aux_node != 0) {
            val = (ling_val*)aux_node->data;
            os_sprintf(buffer, "%s ", val->name);
            serial_debug(buffer, DEBUG_1);
            aux_node = aux_node->next;
        }
        serial_debug("\n", DEBUG_1);
        node = node->next;
    }

    serial_debug("Rules\n=====\n", DEBUG_1);
    node = engine->rules->head;
    condition* cond;
    while(node != 0) {
        fuzzy_rule* rule = node->data;
        aux_node = rule->antecedent->head;
        while(aux_node != 0) {
            cond = aux_node->data;
            os_sprintf(buffer, "if %s is %s %d ", cond->variable->name,
                       cond->value->name, cond->op);
            serial_debug(buffer, DEBUG_1);
            aux_node = aux_node->next;
        }

        os_sprintf(buffer, "then %s is %s  |  result: ",
                   rule->consequent->variable->name,
                   rule->consequent->value->name);
        ftoa(rule->result, buffer);
        os_sprintf(buffer_tmp, "%s consequent value: ", buffer);
        ftoa(rule->consequent->result, buffer_tmp);
        serial_debug(buffer, DEBUG_1);
        node = node->next;
    }
}

uint8_t
register_value(fuzzy_engine* engine, char*name, int id,
               double value)
{
    linked_list_node* node = engine->ling_vars->head;
    ling_var* var;
    while(node != 0) {
        var = (ling_var*)node->data;
        if(name != 0 && strcmp(var->name, name) == 0) {
            var->value = value;
            return 1;
        } else if(id >= 0 && var->id == id) {
            var->value = value;
            return 1;
        }
        node = node->next;
    }
    return 0;
}

uint8_t
register_value_by_name(fuzzy_engine* engine, char* name, double value)
{
    return register_value(engine, name, -1, value);
}

uint8_t
register_value_by_id(fuzzy_engine* engine, int id, double value)
{
    return register_value(engine, 0, id, value);
}

double trapezium_mf(double input, ling_val* value)
{
    double a, b, c, d;
    a = value->a;
    b = value->b;
    c = value->c;
    d = value->d;

    /* if outside of boundries */
    if(input <= a && input >= d) {
        return 0;
    }
    /* if on the ascending side */
    if(input > a && input <= b) {
        return (input - a) / (b - a);
    }
    /* if on the top side */
    if(input > b && input <= c) {
        return 1;
    }
    /* if on the descending side */
    if(input > c && input <= d) {
        return (d - input) / (d - c);
    }
    return 0;
}

double max(double a, double b)
{
    return a > b ? a : b;
}

double min(double a, double b)
{
    return a < b ? a: b;
}


void evaluate_rules(fuzzy_engine* engine)
{
    linked_list_node* rule_node;
    linked_list_node* condition_node;
    condition* cond;
    double tmp_res;
    fuzzy_op op = NONE;

    rule_node = engine->rules->head;
    while(rule_node != 0) {
        op = NONE;
        fuzzy_rule* rule = rule_node->data;
        condition_node = rule->antecedent->head;

        while(condition_node != 0) {
            cond = condition_node->data;
            tmp_res = trapezium_mf(cond->variable->value, cond->value);

            if(op == AND) {
                rule->result = min(rule->result, tmp_res);
            } else if(op == OR) {
                rule->result = max(rule->result, tmp_res);
            } else {
                rule->result = tmp_res;
            }

            op = cond->op;
            condition_node = condition_node->next;
        }
        rule->consequent->result = max(rule->consequent->result, rule->result);

        rule_node = rule_node->next;
    }
}

// insertion sort

void order_consequents_geometrically(rule_consequent** consequent_list,
                                            int length)
{
    char buffer[100];
    os_sprintf(buffer, "length: %d", length);
    int i, j;
    rule_consequent* temp;
    for(i = 1; i < length; i++){
        temp = consequent_list[i];
        j = i - 1;
        while(j >= 0 && temp->value->a < consequent_list[j]->value->a) {
            os_sprintf(buffer, "lol? : %d and %d", (int)temp->value->a,
                       (int)consequent_list[j]->value->a);
            consequent_list[j + 1] = consequent_list[j];
            j = j - 1;
        }
        consequent_list[j + 1] = temp;
    }
}

double get_x_for_y_on_line(double x1, double x2, double y, int dir)
{
    if(x1 == x2) {
        return x1;
    }
    double slope = 1 / (x2 - x1) * dir;
    return (slope * x1 + y) / slope;
}

point*
get_intersection_point(point p1, point p2, point p3, point p4)
{
    double x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
    double y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;
    double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    // If d is zero, there is no intersection
    if (d == 0) {
        return 0;
    }

    // Get the x and y
    double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
    double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
    double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

    // Check if the x and y coordinates are within both lines
    if(x < min(x1, x2) || x > max(x1, x2) ||
            x < min(x3, x4) || x > max(x3, x4)) {
        return 0;
    }
    if(y < min(y1, y2) || y > max(y1, y2) ||
            y < min(y3, y4) || y > max(y3, y4)) {
        return 0;
    }

    // Return the point of intersection
    point * p = (point*)os_malloc(sizeof(point));
    p->x = x;
    p->y = y;
    return p;
}

point* get_raw_polygon_points(rule_consequent** consequent_list,
                                     uint8_t length)
{
    rule_consequent* consequent;
    ling_val* val;
    point* points = (point*)os_malloc(length * 4 * sizeof(point));
    uint8_t i;
    uint8_t count = 0;
    for(i = 0; i < length; i++) {
        consequent = consequent_list[i];
        val = consequent->value;

        points[count].x = val->a;
        points[count].y = 0;
        // printf("%.2f,%.2f;", points[count].x, points[count].y);
        count++;

        points[count].x = get_x_for_y_on_line(val->a, val->b,
                                              consequent->result, 1);
        points[count].y = consequent->result;
        // printf("%.2f,%.2f;", points[count].x, points[count].y);
        count++;

        points[count].x = get_x_for_y_on_line(val->c, val->d,
                                              consequent->result, -1);
        points[count].y = consequent->result;
        // printf("%.2f,%.2f;", points[count].x, points[count].y);
        count++;

        points[count].x = val->d;
        points[count].y = 0;
        // printf("%.2f,%.2f\n", points[count].x, points[count].y);
        count++;
    }

    return points;
}


linked_list* get_polygon_points(point* points, uint8_t length)
{
    uint8_t i, j, found;
    point* intersection_point, *tmp_point;
    linked_list* processed_points = new_linked_list();

    for(i = 0; i < length; i++) {
        found = 0;
        for(j = i + 2; j < length - 1; j++) {
            intersection_point = get_intersection_point(
                points[i], points[i + 1], points[j], points[j + 1]);
            if(intersection_point != 0) {
                tmp_point = (point*)os_malloc(sizeof(point));
                tmp_point->x = points[i].x;
                tmp_point->y = points[i].y;
                processed_points->add(processed_points, (void*)tmp_point);

                // the intersection point
                processed_points->add(processed_points, (void*)intersection_point);

                tmp_point = (point*)os_malloc(sizeof(point));
                tmp_point->x = points[j + 1].x;
                tmp_point->y = points[j + 1].y;
                processed_points->add(processed_points, (void*)tmp_point);
                found = 1;
                break;
            }
        }

        if(found == 0) {
            tmp_point = (point*)os_malloc(sizeof(point));
            tmp_point->x = points[i].x;
            tmp_point->y = points[i].y;
            processed_points->add(processed_points, (void*)tmp_point);
        } else {
            i = i + 3;
        }
    }

    return processed_points;
}


point* get_centroid(linked_list* points)
{
    double signed_area = 0.0;
    double paritial_signed_area = 0.0;
    double x0, y0, x1, y1;
    x0 = y0 = x1 = y1 = 0.0;
    linked_list_node* node = points->head;
    point* current_point, *next_point, *centroid;
    centroid = (point*)os_malloc(sizeof(point));
    while(node->next != 0) {
        current_point = (point*)node->data;
        next_point = (point*)((linked_list_node*)node->next)->data;

        x0 = current_point->x;
        y0 = current_point->y;
        x1 = next_point->x;
        y1 = next_point->y;

        paritial_signed_area = x0*y1 - x1*y0;
        signed_area += paritial_signed_area;
        centroid->x += (x0 + x1) * paritial_signed_area;
        centroid->y += (y0 + y1) * paritial_signed_area;

        node = node->next;
    }


    current_point = (point*)points->tail->data;
    next_point = (point*)points->head->data;

    x0 = current_point->x;
    y0 = current_point->y;
    x1 = next_point->x;
    y1 = next_point->y;

    paritial_signed_area = x0*y1 - x1*y0;
    signed_area += paritial_signed_area;
    centroid->x += (x0 + x1) * paritial_signed_area;
    centroid->y += (y0 + y1) * paritial_signed_area;

    if(signed_area == 0) {
        os_free(centroid);
        return 0;
    }

    signed_area *= 0.5;
    centroid->x /= (6.0 * signed_area);
    centroid->y /= (6.0 * signed_area);

    return centroid;
}

point* defuzzify(fuzzy_engine* engine)
{
    uint8_t consequents_no = engine->consequents->length;
    rule_consequent** consequent_list = (rule_consequent**)os_malloc(
        consequents_no * sizeof(rule_consequent*));
    uint8_t count = 0, i = 0;
    linked_list_node* node = engine->consequents->head;
    while(node != 0) {
        consequent_list[count] = (rule_consequent*)node->data;
        node = node->next;
        count++;
    }

    order_consequents_geometrically(consequent_list, consequents_no);
    point* raw_points = get_raw_polygon_points(consequent_list, consequents_no);
    // debugging
    // for(i = 0; i < consequents_no * 4; i++) {
    //     printf("%.2f, %.2f\n", raw_points[i].x, raw_points[i].y);
    // }
    linked_list* points = get_polygon_points(raw_points, consequents_no * 4);
    // debugging
    // node = points->head;
    // point * p;
    // while(node != 0) {
    //     p = (point*)node->data;
    //     printf("%.2f, %.2f\n", p->x, p->y);
    //     node = node->next;
    // }

    point* centroid = get_centroid(points);

    // free everything
    linked_list_node *aux_node;
    node = points->head;
    while(node != 0) {
        aux_node = node->next;
        os_free(node->data);
        os_free(node);
        node = aux_node;
    }
    os_free(points);
    os_free(raw_points);
    os_free(consequent_list);

    // reset rules
    linked_list_node *rule_node = engine->rules->head;
    while(rule_node != 0) {
        fuzzy_rule* rule = rule_node->data;
        rule->consequent->result = 0;
        rule->result = 0;

        rule_node = rule_node->next;
    }

    return centroid;
}

point* run_fuzzy(fuzzy_engine* engine)
{
    evaluate_rules(engine);
    return defuzzify(engine);
}
