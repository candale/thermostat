import json

with open("res.json", "r") as file_:
    data = json.load(file_)
    data = sorted(data, key=lambda x: x['tmp_err'])

    with open("experiment.dat", "w") as write_file:
        write_file.write("# x y\n")
        count = 0
        last = data[0]['tmp_err']
        for record in data:
            if record['tmp_err'] != last:
                write_file.write("\n")
                last = record["tmp_err"]

            write_file.write('{} {} {} {}\n'.format(
                record['tmp_err'], record['humidity'],
                record['slope'], record['x']))
            count += 1
