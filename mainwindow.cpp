#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <string>
#include <iostream>
#include <vector>
#include "sqlite3.h"
#include <fstream>

using namespace std;

class Converter {
private:
    string path;
    string db_name;
    string db_path;
    vector<string> columns;
    vector<string> column_names;
    vector<string> data;

    string get_db_name()
    {
        int startPos;
        int endPos;

        startPos = this->path.find_last_of("\\");
        endPos = this->path.find_last_of(".");

        if (startPos == -1 && endPos == -1) {
            return "";
        }

        return this->path.substr(startPos + 1, endPos - startPos - 1);
    }

    string make_db_path()
    {
        int pointPos = this->path.find_last_of(".");
        db_path = this->path.substr(0, pointPos + 1) + "db";

        return db_path;
    }

    int read_columns()
    {
        ifstream file(this->path);
        string columns;
        int endPos = 0;

        if (!file)
        {
            return 0;
        }

        getline(file, columns);
        while (endPos != -1)
        {
            endPos = columns.find(",");
            if (endPos == -1) {
                this->columns.push_back(columns);
                break;
            }

            this->columns.push_back(columns.substr(0, endPos));
            columns = columns.substr(endPos + 1);
        }

        return 0;
    }

    int get_column_names()
    {
        int col_count = this->columns.size();
        int pos;
        string column;

        for (int i = 0; i < col_count; i++) {
            column = this->columns[i];
            pos = column.find(" ");
            this->column_names.push_back(column.substr(0, pos));
        }

        return 0;
    }

    int createDB()
    {
        sqlite3* DB;

        int exit = sqlite3_open(this->db_path.c_str(), &DB);
        sqlite3_close(DB);
        return 0;
    }

    int createTable()
    {
        sqlite3* DB;
        char* message_error;
        int col_count = this->columns.size();
        string sql = "CREATE TABLE IF NOT EXISTS " + this->db_name + "(";

        for (int i = 0; i < col_count; i++) {
            if (i == col_count - 1) {
                sql += this->columns[i] + ")" + ";";
                break;
            }

            sql += this->columns[i] + ", ";
        }

        try
        {
            int exit = sqlite3_open(this->db_path.c_str(), &DB);
            exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &message_error);
            if (exit != SQLITE_OK) {
                cout << "Error" << endl;
                sqlite3_free(message_error);
                return 0;
            }
            cout << "Table created Successfully" << endl;
            sqlite3_close(DB);
        }
        catch (const exception& e)
        {
            cerr << e.what();
        }
        return 0;
    }

    int get_data()
    {
        ifstream file(this->path);
        string data;

        if (!file) {
            return 0;
        }

        getline(file, data);

        while (getline(file, data)) {
            if (data.find("\n") != -1) {
                data.replace(data.size() - 1, 1, "");
            }
            this->data.push_back(data);
        }

        return 0;
    }

    int insertData()
    {
        sqlite3* DB;
        char* error_message;
        int col_count = this->column_names.size();
        string sql_temp = "INSERT INTO " + this->db_name + " (";
        string sql = "";
        int data_count = this->data.size();

        for (int i = 0; i < col_count; i++) {
            if (i == col_count - 1) {
                sql_temp += this->column_names[i] + ")" + "VALUES (";
                break;
            }
            sql_temp += this->column_names[i] + ", ";
        }

        for (int i = 0; i < data_count; i++) {
            sql += sql_temp;
            sql += this->data[i];
            sql += "); ";
        }


        int exit = sqlite3_open(this->db_path.c_str(), &DB);
        exit = sqlite3_exec(DB, sql.c_str(), NULL, 0, &error_message);
        if (exit != SQLITE_OK) {
            cout << "Error" << endl;
            sqlite3_free(error_message);
            return 0;
        }

        cout << "Records inserted Successfully!" << endl;
        return 0;
    }

public:
    Converter(string path)
    {
        this->path = path;
        this->db_name = get_db_name();
        this->db_path = make_db_path();
        createDB();
        read_columns();
        get_column_names();
        createTable();
        get_data();
        insertData();

    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->textEdit->setAlignment(Qt::AlignCenter);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString path = ui->textEdit->toPlainText();
    string path_str = path.toStdString();
    Converter converter(path_str);
}

