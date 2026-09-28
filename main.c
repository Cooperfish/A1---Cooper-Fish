#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define BUFFER_SIZE 1024

int extractIPv4(const char* str, unsigned long* outAddress, int* outPort);

static int isTokenChar(char c)
{
    return isdigit((unsigned char)c) || c == '.' || c == ':';
}

static int parseNumber(const char* str, int* pos, int end, int maxDigits, long maxValue, long* value)
{
    int digits = 0;
    long result = 0;
    int p = *pos;

    while (p < end && isdigit((unsigned char)str[p])) {
        if (digits == maxDigits) {
            return 0;
        }
        result = result * 10 + (str[p] - '0');
        digits++;
        p++;
    }

    if (digits == 0) {
        return 0;
    }
    if (digits > 1 && str[*pos] == '0') {
        return 0;
    }
    if (result > maxValue) {
        return 0;
    }

    *value = result;
    *pos = p;
    return 1;
}

static int validateToken(const char* str, int start, int end, unsigned long* address, int* port)
{
    int pos = start;
    unsigned long addr = 0;
    long value;
    int i;

    for (i = 0; i < 4; i++) {
        if (!parseNumber(str, &pos, end, 3, 255, &value)) {
            return 0;
        }
        addr = (addr << 8) | (unsigned long)value;

        if (i < 3) {
            if (pos >= end || str[pos] != '.') {
                return 0;
            }
            pos++;
        }
    }

    if (pos == end) {
        *address = addr;
        *port = -1;
        return 1;
    }

    if (str[pos] != ':') {
        return 0;
    }
    pos++;

    if (!parseNumber(str, &pos, end, 5, 65535, &value)) {
        return 0;
    }
    if (pos != end) {
        return 0;
    }

    *address = addr;
    *port = (int)value;
    return 1;
}

int extractIPv4(const char* str, unsigned long* outAddress, int* outPort)
{
    int i = 0;
    int start;
    unsigned long address;
    int port;

    *outAddress = 0;
    *outPort = -1;

    if (str == NULL) {
        return 0;
    }

    while (str[i] != '\0') {
        if (!isTokenChar(str[i])) {
            i++;
            continue;
        }

        start = i;
        while (str[i] != '\0' && isTokenChar(str[i])) {
            i++;
        }

        if (validateToken(str, start, i, &address, &port)) {
            *outAddress = address;
            *outPort = port;
            return 1;
        }
    }

    return 0;
}

int main(void)
{
    char line[BUFFER_SIZE];
    unsigned long address;
    int port;

    while (1) {
        printf("Enter a string (or 'END' to quit): ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;
        }

        line[strcspn(line, "\r\n")] = '\0';

        if (strcmp(line, "END") == 0) {
            break;
        }

        if (extractIPv4(line, &address, &port)) {
            printf("Extracted IPv4 address: %lu.%lu.%lu.%lu (decimal value: %lu, port: ",
                   (address >> 24) & 0xFF,
                   (address >> 16) & 0xFF,
                   (address >> 8) & 0xFF,
                   address & 0xFF,
                   address);
            if (port == -1) {
                printf("none)\n");
            } else {
                printf("%d)\n", port);
            }
        } else {
            printf("Invalid input: no valid IPv4 address found\n");
        }
    }

    printf("Program terminated.\n");
    return 0;
}