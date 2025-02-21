/* here we provide a simple entry for asm print */
extern void assembly_print(const char* msg, const int msg_len);

void c_print(const char* msg)
{
    int len = 0;
    while(msg[len]){
        len++; // fetch the length of a string
    }
    assembly_print(msg, len);
}