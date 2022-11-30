struct QEI_t{
    int value;
    int delta;
};

enum QEI_name_t{
    QEI_A_NAME=0,
    QEI_B_NAME=1,
    QEI_C_NAME=2
};

extern struct QEI_t QEI_A, QEI_B, QEI_C;

void QEI_update(void);
void QEI_init(void);
int QEI_get(enum QEI_name_t qei);
double QEI_get_mm(enum QEI_name_t qei);