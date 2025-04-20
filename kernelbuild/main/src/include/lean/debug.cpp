bool go = false;

extern "C" bool checkGo() {
    return go;
}

extern "C" void setGo(bool value) {
    go = value;
}