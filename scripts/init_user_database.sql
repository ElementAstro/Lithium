CREATE TABLE users (
    id          TEXT PRIMARY KEY,
    username    TEXT NOT NULL UNIQUE,
    email       TEXT NOT NULL UNIQUE,
    pswhash     TEXT NOT NULL
);

INSERT INTO users (id, username, email, pswhash)
VALUES ('4bb48865-1a84-49c7-94eb-148ed2f9d7a5', 'admin', 'admin@domain.com', 'admin');
