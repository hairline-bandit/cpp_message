#include <SFML/Network.hpp>
#include "Message.h"
#include "EncryptDecrypt.h"
#include "Utilities.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <regex>

using namespace std;
using namespace sf;

// global variables for use by all functions
string username = "";
string password = "";
string key = "";
vector<Message>* message = new vector<Message>; // all messages
vector<string>* friends = new vector<string>; // all friends
vector<string>* pendingFriends = new vector<string>; // received friend requests
string url = "http://ethanm.pythonanywhere.com"; // web server address

void exit();
void getData();
int update();
string openChat();
void friendsList();
void sendMessage(string);
int createAccount();
int login();
void acceptDecline();
int sendFriend();
bool in(string);

int main() {
    char choice;
    cout << "Login (1) or create an account (2): ";
    cin >> choice;
    int result;
    if (choice == '1') {
        result = login();
        if (result == -2) {
            cout << "User not found" << endl;
            return -1;
        } else if (result == -1) {
            cout << "Incorrect password" << endl;
            return -1;
        } else {
            cout << "Successfully logged in" << endl;
            getData();
        }
    } else {
        result = createAccount();
        if (result == -2) {
            cout << "Illegal characters in that username" << endl;
            return -1;
        } else if (result == -1) {
            cout << "That userame is already taken" << endl;
            return -1;
        } else {
            cout << "Account created successfully" << endl;
        }
    }
    choice = '0';
    while (choice != 'e' && choice != 'E') {
        int status = update(); // see if there's any new information every loop iteration
        if (status < 0) {
            break;
        }
        cout 
        << "Open a chat (1)\n"
        << "Display your friends list (2)\n"
        << "Send a friend request (3)\n"
        << "Accept/decline a friend request (4)\n"
        << "Exit (e)\n";
        cin >> choice;
        status = update();
        if (status < 0) {
            break;
        }
        if (choice == '1') {
            string person = openChat();
            cout << "Go back (b) or send a message (m): ";
            char choice2;
            cin >> choice2;
            if (choice2 == 'm' || choice2 == 'M') {
                sendMessage(person);
            }
        } else if (choice == '2') {
            friendsList();
        } else if (choice == '3') {
            int success = sendFriend();
            if (success == -1) {
                cout << "That user could not be found" << endl;
            }
        } else if (choice == '4') {
            acceptDecline();
        }
    }
    exit();
    return 0;
}

bool in(string f) { // function to see if an item in the pendingFriends vector was already accepted
    for (size_t i = 0; i < friends->size(); ++i) {
        if (friends->at(i) == f) {
            return true;
        }
    }
    return false;
}

int createAccount() { // used to create an account
    Http http;
    http.setHost(url);

    Http::Request request;
    request.setMethod(Http::Request::Post);
    request.setUri("/createacc");

    string user, passw, encKey;
    cout << "Enter your desired username (no spaces or punctuation): ";
    cin >> user;
    cout << "Enter password: ";
    cin >> passw;
    cout << "Enter desired encryption password: ";
    cin >> encKey;
    // send a post request with the required information
    request.setBody("user="+user+"&passw="+passw+"&key="+encKey);

    Http::Response response = http.sendRequest(request);
    string resp = response.getBody(); // the json response (as a string)
    resp = resp.substr(resp.find("resp")+7, resp.substr(resp.find("resp")+7).find("\"")); // get the value from the key "resp"
    if (resp == "illegal character(s)") { // if there's characters that aren't allowed
        return -2;
    } else if (resp == "username taken") { // if someone already has the username
        return -1;
    } else {
        regex last("account created"); // success!
        key = regex_replace(resp, last, "");
        // if the login was successful. the encryption/decryption key is stored on the webserver
        // so it is returned when you login or create an account. for use by the program.
        username = user;
        password = passw;
        return 0;
    }
}

int login() { // for logging in
    Http http;
    http.setHost(url);

    Http::Request request;
    request.setMethod(Http::Request::Post);
    request.setUri("/login");
    

    string user, passw;
    cout << "Enter username: ";
    cin >> user;
    cout << "Enter password: ";
    cin >> passw;
    // post request
    request.setBody("user="+user+"&passw="+passw);

    Http::Response response = http.sendRequest(request);

    string resp = response.getBody();
    resp = resp.substr(resp.find("resp")+7, resp.substr(resp.find("resp")+7).find("\"")); // get only the response portion we care about
    if (resp == "user not found") { // if the username was incorrect
        return -2;
    } else if (resp == "password incorrect") { // if the password was incorrect
        return -1;
    } else {
        // if the login was successful. the encryption/decryption key is stored on the webserver
        // so it is returned when you login or create an account. for use by the program.
        key = resp.substr(resp.find("login successful")+16, resp.substr(resp.find("login successful")+16).find("\""));
        username = user;
        password = passw;
        return 0;
    }
}

void exit() { // for when the user exits the program. saves and encrypts all relevant data
    // used for saving who's your friend
    ofstream fout("data/friends.txt");
    if (fout.fail()) {
        cerr << "Couldn't open friends file" << endl;
        return;
    }
    for (size_t i = 0; i < friends->size(); ++i) {
        fout << friends->at(i);
        if (i != friends->size()-1) {
            fout << endl;
        }
    }
    fout.close();
    // used for saving all messages. every piece of data associated with it is encrypted with the key
    fout.open("data/messages.txt");
    if (fout.fail()) {
        cerr << "Couldn't open messages file" << endl;
        return;
    }
    for (size_t i = 0; i < message->size(); ++i) {
        EncryptDecrypt* encryptor;
        for (int j = 0; j < 4; ++j) {
            switch (j) {
                case 0:
                    encryptor = new EncryptDecrypt(message->at(i).getMessage(), key, 'e');
                    break;
                case 1:
                    encryptor = new EncryptDecrypt(message->at(i).getFrom(), key, 'e');
                    break;
                case 2:
                    encryptor = new EncryptDecrypt(message->at(i).getTo(), key, 'e');
                    break;
                default:
                    encryptor = new EncryptDecrypt(message->at(i).getTimestamp(), key, 'e');
            }
            fout << encryptor->encrypt() << ":";
            delete encryptor;
        }
        if (i != message->size()-1) {
            fout << endl;
        }
        encryptor = nullptr;
    }
    fout.close();
    // for saving pending friend requests sent to the user
    fout.open("data/pending.txt");
    if (fout.fail()) {
        cerr << "Couldn't open pending friends file" << endl;
        return;
    }
    for (size_t i = 0; i < pendingFriends->size(); ++i) {
        if (!in(pendingFriends->at(i))) {
            fout << pendingFriends->at(i);
            if (i != pendingFriends->size()-1) {
                fout << endl;
            }
        }
    }
    fout.close();
}

void getData() { // used for jgetting all data from the text files
    // push friends list to the vector.
    ifstream fin("data/friends.txt");
    if (fin.fail()) {
        cerr << "Couldn't open friends file" << endl;
        return;
    }
    string curr = "";
    while (fin >> curr) {
        friends->push_back(curr);
        curr = "";
    }
    fin.close();
    // pushes new Message objects to the vector. must be decrypted first with the key.
    fin.open("data/messages.txt");
    if (fin.fail()) {
        cerr << "Couldn't open messages file" << endl;
        return;
    }
    while (fin >> curr) {
        message->push_back(Message(curr, key));
        curr = "";
    }
    fin.close();
    // push all pending friend requests to the vector.
    fin.open("data/pending.txt");
    if (fin.fail()) {
        cerr << "Couldn't open pending friends file" << endl;
        return;
    }
    while (fin >> curr) {
        pendingFriends->push_back(curr);
        curr = "";
    }
    fin.close();
}

int update() { // used to get new messages/friend requests from the server
    Http http;
    http.setHost(url);

    Http::Request request;
    request.setMethod(Http::Request::Post);
    request.setUri("/refresh");
    

    request.setBody("user="+username+"&passw="+password);

    Http::Response response = http.sendRequest(request);
    string resp = response.getBody();
    resp = resp.substr(resp.find("resp")+7, resp.size()-1);
    string data = resp;
    // if the data sent in the post request is wrong
    if (resp == "user not found\"}") {
        return -2;
    } else if (resp == "wrong password\"}") {
        return -1;
    } else {
        // split the data into individual portions (friend requests or messages)
        vector<string> everything = split(data, regex("`"));
        for (size_t i = 0; i < everything.size(); ++i) {
            string type;
            regex rgx("\\{.*");
            type = regex_replace(everything.at(i), rgx, ""); // only the type of data
            if (type == "msg") { // messages
                message->push_back(Message(everything.at(i))); // the Message class handles getting the data
                cout << "New message from " << message->at(message->size()-1).getFrom() << endl; // alert the user
            } else if (type == "friendreq") { // if someone sent you a friend request
                // get only whoever sent it to you and push to pendingFriends
                string from = everything.at(i).substr(everything.at(i).find(".from")+10, everything.at(i).substr(everything.at(i).find(".from")+10, everything.at(i).size()-1).find('\\'));
                pendingFriends->push_back(from);
                cout << "New friend request from " << from << endl;
            } else if (type == "acceptreq") {// if someone accepted your friend request
                // get only whoever accepted it and push to your friends list
                string newFriend = everything.at(i).substr(everything.at(i).find(".to")+8, everything.at(i).substr(everything.at(i).find(".to")+8, everything.at(i).size()-1).find('\\'));
                friends->push_back(newFriend);
                cout << newFriend << " accepted your friend request" << endl;
            } else if (type == "declinereq") { // same thing but if someone declined your request
                string notFriend = everything.at(i).substr(everything.at(i).find(".to")+8, everything.at(i).substr(everything.at(i).find(".to")+8, everything.at(i).size()-1).find('\\'));
                cout << notFriend << " declined your friend request" << endl;
            }
        }
    }
    return 0;
}

void friendsList() { // print out all of your friends
    cout << "Here is your friends list" << endl;
    for (size_t i = 0; i < friends->size(); ++i) {
        cout << friends->at(i) << endl;
    }
}

string openChat() { // print all previous messages between 2 users
    friendsList(); // show all your friends
    string person;
    cout << "What chat would you like to open? ";
    cin >> person;
    for (size_t i = 0; i < message->size(); ++i) {
        if (message->at(i).getFrom() == person || message->at(i).getTo() == person) { // print out only messages where you sent or received
            cout << message->at(i).getFrom() << ":" << endl;
            cout << message->at(i).getMessage() << endl << endl;
        }
    }
    return person;
}

void acceptDecline() { // accept or decline a friend request
    for (size_t i = 0; i < pendingFriends->size(); ++i) {
        cout << pendingFriends->at(i) << endl;
    } // display all of the pending requests
    string choice;
    cout << "Which user would you like to accept/decline? ";
    cin >> choice;
    bool exists = false;
    for (size_t i = 0; i < pendingFriends->size(); ++i) {
        if (pendingFriends->at(i) == choice) {
            exists = true;
        }
    }
    if (!exists) {
        cout << "That user hasn't sent you a friend request." << endl << endl;
        return;
    }
    char ad;
    cout << "Accept (1) or decline (2): ";
    cin >> ad;
    if (ad != '1' && ad != '2') {
        cout << "That was not a choice" << endl << endl;
        return;
    }
    Http http;
    http.setHost(url);
    // alert the webserver/other user that you are now friends.
    Http::Request request;
    request.setMethod(Http::Request::Post);
    if (ad == '1') {
        request.setUri("/acceptfriend");
        friends->push_back(choice);
    } else {
        request.setUri("/declinefriend");
    }
    

    request.setBody("user="+username+"&passw="+password+"&other="+choice+"&timestamp="
    +to_string(chrono::steady_clock::now().time_since_epoch().count()));

    Http::Response response = http.sendRequest(request);

    string resp = response.getBody();
    // don't need to deal with the response.
}

void sendMessage(string user) { // send a message to another user
    Http http;
    http.setHost(url);

    Http::Request request;
    request.setMethod(Http::Request::Post);
    request.setUri("/send");
    
    // get the message to send (need to use getline so that we can read in spaces)
    // clear characters that would mess it up using cin.ignore()
    string text;
    cin.ignore();
    getline(cin, text);

    // we can't encrypt if the length is longer than 64 so it's the max message size.
    if (text.size() > 64) {
        cout << "Message over 64 characters!" << endl;
        return;
    }
    // timestamp the message
    int timestamp = chrono::duration_cast<chrono::seconds>(chrono::steady_clock::now().time_since_epoch()).count();

    request.setBody("from="+username+"&to="+user+"&timestamp="+to_string(timestamp)+"&msg="+text+"&passw="+password);

    Http::Response response = http.sendRequest(request);
    string resp = response.getBody();
    resp = resp.substr(resp.find("resp")+7, resp.substr(resp.find("resp")+7).find("\""));
    // if anything went wrong
    if (resp == "user not found") {
        cout << "User not found" << endl;
    } else if (resp == "wrong password") {
        cout << "Wrong password" << endl;
    } else if (resp == "recipient not found") {
        cout << "The other user does not exist" << endl;
    } else if (resp == "you are not friends") {
        cout << "The other user is not your friend" << endl;
    } else {
        message->push_back(Message(text, username, user, to_string(timestamp)));
    }
}

int sendFriend() { // send a friend request to another user
    Http http;
    http.setHost(url);

    Http::Request request;
    request.setMethod(Http::Request::Post);
    request.setUri("/friendreq");
    

    string to;
    cout << "Enter the username of the person you'd like to send a friend request to: ";
    cin >> to;

    string timestamp = to_string(chrono::steady_clock::now().time_since_epoch().count());

    request.setBody("from="+username+"&to="+to+"&passw="+password+"&timestamp="+timestamp);

    Http::Response response = http.sendRequest(request);
    string resp = response.getBody();
    resp = resp.substr(resp.find("resp")+7, resp.substr(resp.find("resp")+7).find("\""));
    // in case something went wrong
    if (resp == "user not found") {
        return -3;
    } else if (resp == "wrong password") {
        return -2;
    } else if (resp == "recipient not found") {
        return -1;
    }
    return 0;
}   