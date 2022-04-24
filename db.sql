DROP TABLE IF EXISTS Patients CASCADE;
DROP TABLE IF EXISTS Studies CASCADE;

CREATE TABLE Patients (
    id serial,
    name VARCHAR(20) NOT NULL,
    surname VARCHAR(20) NOT NULL,
    pesel VARCHAR(11),
    PRIMARY KEY(id));

CREATE TABLE Studies (
    patient_id serial,
    type VARCHAR(20) NOT NULL,
    date DATE NOT NULL,
    result VARCHAR(20) NOT NULL,
    FOREIGN KEY (patient_id) REFERENCES Patients(id) ON UPDATE CASCADE
);

INSERT INTO Patients (name, surname, pesel) VALUES ('Maciej', 'Kowalski', '94072163897');
INSERT INTO Patients (name, surname, pesel) VALUES ('Janusz', 'Nowak', '58060398959');
INSERT INTO Patients (name, surname, pesel) VALUES ('Krystian', 'Wesołowski', '67012969347');
INSERT INTO Patients (name, surname, pesel) VALUES ('Kazimierz', 'Zamojski', '77032699391');
INSERT INTO Patients (name, surname, pesel) VALUES ('Stefan', 'Kamiński', '85080513242');
INSERT INTO Patients (name, surname, pesel) VALUES ('Władysław', 'Kowalski', '65122277196');

INSERT INTO Studies (patient_id, type, date, result) VALUES (1, 'HIV Test', '2022-04-12', 'Negative');
INSERT INTO Studies (patient_id, type, date, result) VALUES (1, 'Throat Cancer Test', '2009-05-12', 'Negative');

INSERT INTO Studies (patient_id, type, date, result) VALUES (2, 'HIV Test', '2022-04-12', 'Negative');
INSERT INTO Studies (patient_id, type, date, result) VALUES (2, 'Throat Cancer Test', '2009-05-12', 'Negative');

INSERT INTO Studies (patient_id, type, date, result) VALUES (3, 'HIV Test', '2011-08-14', 'Negative');
INSERT INTO Studies (patient_id, type, date, result) VALUES (3, 'HIV Test', '2009-05-12', 'Negative');

INSERT INTO Studies (patient_id, type, date, result) VALUES (4, 'COVID Test', '2022-03-09', 'Positive');
INSERT INTO Studies (patient_id, type, date, result) VALUES (4, 'Leukemia Test', '2009-05-12', 'Negative');

INSERT INTO Studies (patient_id, type, date, result) VALUES (5, 'HIV Test', '2022-04-12', 'Positive');
INSERT INTO Studies (patient_id, type, date, result) VALUES (5, 'Throat Cancer Test', '2008-05-12', 'Negative');

INSERT INTO Studies (patient_id, type, date, result) VALUES (6, 'HIV Test', '2022-04-12', 'Positive');
INSERT INTO Studies (patient_id, type, date, result) VALUES (6, 'Bone Cancer Test', '2007-06-10', 'Negative');

-- select * from Patients

-- SELECT Studies.* FROM Studies JOIN Patients ON Patients.id = Studies.patient_id WHERE patient_id = 1 OR patient_id = 2;

SELECT Patients.*, Studies.* FROM Studies JOIN Patients ON Patients.id = Studies.patient_id;
