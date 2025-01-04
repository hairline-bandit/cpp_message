from flask import Flask, render_template, request
import hashlib, string

app = Flask(__name__)

@app.route("/createacc", methods=["GET", "POST"])
def createacc():
    if request.method == "POST":
        user = request.values["user"]
        passw = request.values["passw"]
        encrypt_key = request.values["key"]
        encrypt_key = hashlib.sha256(encrypt_key[0:len(encrypt_key)//2].encode("utf-8")).hexdigest() + hashlib.sha256(encrypt_key[len(encrypt_key)//2:].encode("utf-8")).hexdigest()
        users = []
        with open("database/users.txt", "r+") as f:
            users = f.readlines()
        if " " in user or any([i in string.punctuation for i in user]):
            return {"resp": "illegal character(s)"}
        user = hashlib.sha256(user.encode("utf-8")).hexdigest()
        users = [i.split()[0] for i in users]
        if user in users:
            return {"resp": "username taken"}
        with open("database/users.txt", "a+") as f:
            f.write(user + " " + hashlib.sha256(passw.encode("utf-8")).hexdigest() + " " + encrypt_key + "\n")
        with open("users/" + user + ".txt", "a+") as f:
            pass
        with open("users/" + user + "_friends.txt", "a+") as f:
            pass
        return {"resp": "account created"+encrypt_key}
    else:
        return render_template("createacc.html")

@app.route("/login", methods=["GET", "POST"])
def login():
    if request.method == "POST":
        user = request.values["user"]
        user = hashlib.sha256(user.encode("utf-8")).hexdigest()
        passw = request.values["passw"]
        passw = hashlib.sha256(passw.encode("utf-8")).hexdigest()
        users = []
        passwords = []
        keys = []
        with open("database/users.txt", "r+") as f:
            a = f.readlines()
            users = [i.split()[0] for i in a]
            passwords = [i.split()[1] for i in a]
            keys = [i.split()[2] for i in a]
        if user not in users:
            return {"resp": "user not found"}
        if passwords[users.index(user)] != passw:
            return {"resp": "password incorrect"}
        return {"resp": "login successful" + keys[users.index(user)]}
    else:
        return render_template("login.html")

@app.route("/send", methods=["GET", "POST"])
def send():
    if request.method == "POST":
        frm = request.values["from"]
        to = request.values["to"]
        timestamp = request.values["timestamp"]
        msg = request.values["msg"]
        passw = request.values["passw"]
        frm_hash = hashlib.sha256(frm.encode("utf-8")).hexdigest()
        to_hash = hashlib.sha256(to.encode("utf-8")).hexdigest()
        passw = hashlib.sha256(passw.encode("utf-8")).hexdigest()
        users = []
        passwords = []
        friends1 = []
        friends2 = []
        with open("database/users.txt", "r+") as f:
            a = f.readlines()
            users = [i.split()[0] for i in a]
            passwords = [i.split()[1] for i in a]
        if frm_hash not in users:
            return {"resp": "user not found"}
        if passwords[users.index(frm_hash)] != passw:
            return {"resp": "wrong password"}
        if to_hash not in users:
            return {"resp": "recipient not found"}
        with open("users/" + frm_hash + "_friends.txt", "r+") as f:
            friends1 = [i.strip() for i in f.readlines()]
        with open("users/" + to_hash + "_friends.txt", "r+") as f:
            friends2 = [i.strip() for i in f.readlines()]
        if frm_hash not in friends2 or to_hash not in friends1:
            return {"resp": "you are not friends"}
        with open("users/" + to_hash + ".txt", "a+") as f:
            f.write("msg{\".from\":\"" + frm + "\", \".to\":\"" + to + "\", \".time\":" + str(timestamp) + ", \".message\":\"" + msg + "\"}\n")
        return {"resp": "success"}
    else:
        return render_template("send.html")

@app.route("/refresh", methods=["GET", "POST"])
def refresh():
    if request.method == "POST":
        user = request.values["user"]
        passw = request.values["passw"]
        user = hashlib.sha256(user.encode("utf-8")).hexdigest()
        passw = hashlib.sha256(passw.encode("utf-8")).hexdigest()
        users = []
        passwords = []
        with open("database/users.txt", "r+") as f:
            a = f.readlines()
            users = [i.split()[0] for i in a]
            passwords = [i.split()[1] for i in a]
        if user not in users:
            return {"resp": "user not found"}
        if passwords[users.index(user)] != passw:
            return {"resp": "wrong password"}
        data = ""
        with open("users/" + user + ".txt", "r+") as f:
            a = [i.rstrip() for i in f.readlines()]
            a.append(" ")
            data = "`".join(a)
        with open("users/" + user + ".txt", "w") as f:
            pass
        return {"resp": data}
    else:
        return render_template("refresh.html")

@app.route("/friendreq", methods=["GET", "POST"])
def friendreq():
    if request.method == "POST":
        frm = request.values["from"]
        to = request.values["to"]
        passw = request.values["passw"]
        timestamp = request.values["timestamp"]
        frm_hash = hashlib.sha256(frm.encode("utf-8")).hexdigest()
        to_hash = hashlib.sha256(to.encode("utf-8")).hexdigest()
        passw = hashlib.sha256(passw.encode("utf-8")).hexdigest()
        users = []
        passwords = []
        with open("database/users.txt", "r+") as f:
            a = f.readlines()
            users = [i.split()[0] for i in a]
            passwords = [i.split()[1] for i in a]
        if frm_hash not in users:
            return {"resp": "user not found"}
        if passwords[users.index(frm_hash)] != passw:
            return {"resp": "wrong password"}
        if to_hash not in users:
            return {"resp": "recipient not found"}
        with open("users/" + to_hash + ".txt", "a+") as f:
            f.write("friendreq{\".from\":\"" + frm + "\", \".to\":\"" + to + "\", \".time\":" + timestamp + "}\n")
        with open("database/pending.txt", "a+") as f:
            f.write(frm_hash + " " + to_hash + "\n")
        return {"resp": "success"}
    else:
        return render_template("friendreq.html")
# need a file called database/pending.txt that has pending friend requests set up as "frm to"
@app.route("/acceptfriend", methods=["GET", "POST"])
def acceptfriend():
    if request.method == "POST":
        user = request.values["user"]
        other = request.values["other"]
        passw = request.values["passw"]
        timestamp = request.values["timestamp"]
        passw = hashlib.sha256(passw.encode("utf-8")).hexdigest()
        user_hash = hashlib.sha256(user.encode("utf-8")).hexdigest()
        other_hash = hashlib.sha256(other.encode("utf-8")).hexdigest()
        pending = []
        users = []
        passwords = []
        with open("database/users.txt", "r+") as f:
            a = f.readlines()
            users = [i.split()[0] for i in a]
            passwords = [i.split()[1] for i in a]
        if user_hash not in users:
            return {"resp": "user not found"}
        if passwords[users.index(user_hash)] != passw:
            return {"resp": "wrong password"}
        if other_hash not in users:
            return {"resp": "recipient not found"}
        with open("database/pending.txt", "r+") as f:
            pending = [i.strip() for i in f.readlines()]
        if (other_hash + " " + user_hash) not in pending:
            return {"resp": "friendreq not found"}
        with open("users/" + other_hash + ".txt", "a+") as f:
            f.write("acceptreq{\".from\":\"" + other + "\", \".to\":\"" + user + "\" ,\".timestamp\":" + timestamp + "}\n")
        with open("users/" + user_hash + "_friends.txt", "a+") as f:
            f.write(other_hash + "\n")
        with open("users/" + other_hash + "_friends.txt", "a+") as f:
            f.write(user_hash + "\n")
        pending.pop(pending.index((other_hash + " " + user_hash)))
        with open("database/pending.txt", "w+") as f:
            for i in pending:
                f.write(i + "\n")
        return {"resp": "success"}
    else:
        return render_template("acceptfriend.html")

@app.route("/declinefriend", methods=["GET", "POST"])
def declinefriend():
    if request.method == "POST":
        user = request.values["user"]
        other = request.values["other"]
        passw = request.values["passw"]
        timestamp = request.values["timestamp"]
        passw = hashlib.sha256(passw.encode("utf-8")).hexdigest()
        user_hash = hashlib.sha256(user.encode("utf-8")).hexdigest()
        other_hash = hashlib.sha256(other.encode("utf-8")).hexdigest()
        pending = []
        users = []
        passwords = []
        with open("database/users.txt", "r+") as f:
            a = f.readlines()
            users = [i.split()[0] for i in a]
            passwords = [i.split()[1] for i in a]
        if user_hash not in users:
            return {"resp": "user not found"}
        if passwords[users.index(user_hash)] != passw:
            return {"resp": "wrong password"}
        if other_hash not in users:
            return {"resp": "recipient not found"}
        with open("database/pending.txt", "r+") as f:
            pending = [i.strip() for i in f.readlines()]
        if (other_hash + " " + user_hash) not in pending:
            return {"resp": "friendreq not found"}
        with open("users/" + other_hash + ".txt", "a+") as f:
            f.write("declinereq{\".from\":\"" + other + "\", \".to\":\"" + user + "\", \".timestamp\":" + timestamp + "}\n")
        pending.pop(pending.index((other_hash + " " + user_hash)))
        with open("database/pending.txt", "w+") as f:
            for i in pending:
                f.write(i + "\n")
        return {"resp": "success"}
    else:
        return render_template("declinefriend.html")

if __name__ == "__main__":
    app.run(debug=True)
