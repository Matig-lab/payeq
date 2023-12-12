#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define check_ptr(ptr)                                                         \
    if (!ptr) {                                                                \
        fprintf(stderr, "error: Could not allocate pointer\n");                \
        exit(1);                                                               \
    }

#define PERSON_MAX_NAME_SIZE 64
#define VECTOR_INITAL_CAP    16

struct person {
    char *name;
    float paid, debt, credit;
};

struct person *person_generate(char *name, float paid) {
    if (paid < 0) {
        fprintf(stderr, "warning: Initial payment of a person could not be a "
                        "negative number. Setting to 0.\n");
    }
    name[strnlen(name, PERSON_MAX_NAME_SIZE)] =
        '\0'; // Make sure this character exists
    struct person *np = malloc(sizeof(*np));
    check_ptr(np);
    np->name = strndup(name, PERSON_MAX_NAME_SIZE);
    check_ptr(np->name);
    np->paid   = paid;
    np->debt   = 0.0;
    np->credit = 0.0;
    return np;
}

void person_free(struct person *p) {
    if (!p) return;
    if (p->name) free(p->name);
    free(p);
}

struct person_vec {
    struct person **list;
    size_t size, capacity;
};

struct person_vec *persons_vec_alloc(void) {
    struct person_vec *p_vec = malloc(sizeof(*p_vec));
    p_vec->list = malloc(sizeof(struct person *) * VECTOR_INITAL_CAP);
    check_ptr(p_vec->list);
    p_vec->capacity = VECTOR_INITAL_CAP;
    p_vec->size     = 0;
    return p_vec;
}

void persons_vec_free(struct person_vec *p_vec) {
    if (!p_vec) return;
    for (size_t i = 0; i < p_vec->size; i++) {
        person_free(p_vec->list[i]);
    }
    free(p_vec->list);
    free(p_vec);
}

void person_vec_push(struct person_vec *p_vec, struct person *p) {
    if (!p_vec || !p) {
        return;
    }
    if (p_vec->capacity <= p_vec->size + 1) {
        p_vec->capacity = p_vec->capacity == 0 ? 1 : p_vec->capacity * 2;
        p_vec->list     = realloc(p_vec->list, p_vec->capacity);
        check_ptr(p_vec->list);
    }
    p_vec->list[p_vec->size++] = p;
}

struct payment_record {
    struct person *from, *to;
    float quantity;
};

struct payment_record *payment_record_generate(struct person *from,
                                               struct person *to,
                                               float quantity) {
    if (!from || !to) return NULL;
    struct payment_record *pr = malloc(sizeof(*pr));
    check_ptr(pr);

    // In both cases stores a reference to a person struct. They're borrowed.
    pr->from = from;
    pr->to   = to;

    pr->quantity = quantity;
    return pr;
}

void payment_record_free(struct payment_record *pay_rec) { free(pay_rec); }

struct payment_record_vec {
    struct payment_record **list;
    size_t size, capacity;
};

struct payment_record_vec *payment_record_vec_alloc(void) {
    struct payment_record_vec *p_vec = malloc(sizeof(*p_vec));
    check_ptr(p_vec);
    p_vec->list = malloc(sizeof(struct payment_record *) * VECTOR_INITAL_CAP);
    check_ptr(p_vec->list);
    p_vec->size     = 0;
    p_vec->capacity = VECTOR_INITAL_CAP;
    return p_vec;
}

void payment_record_vec_free(struct payment_record_vec *pr_vec) {
    if (!pr_vec) return;
    for (size_t i = 0; i < pr_vec->size; i++) {
        payment_record_free(pr_vec->list[i]);
    }
    free(pr_vec->list);
    free(pr_vec);
}

void payment_record_vec_push(struct payment_record_vec *pr_vec,
                             struct payment_record *pr) {
    if (!pr_vec || !pr) {
        return;
    }
    if (pr_vec->capacity <= pr_vec->size + 1) {
        pr_vec->capacity = pr_vec->capacity == 0 ? 1 : pr_vec->capacity * 2;
        pr_vec->list     = realloc(pr_vec->list, pr_vec->capacity);
        check_ptr(pr_vec->list);
    }
    pr_vec->list[pr_vec->size++] = pr;
}

struct payeq {
    struct person_vec *persons;
    struct payment_record_vec *payment_records;
    float total_debts, avg_payment;
};

struct payeq *payeq_alloc(void) {
    struct payeq *pq = malloc(sizeof(*pq));
    check_ptr(pq);
    pq->persons         = persons_vec_alloc();
    pq->payment_records = payment_record_vec_alloc();
    pq->total_debts     = 0.0;
    pq->avg_payment     = 0.0;
    return pq;
}

void payeq_free(struct payeq *payeq) {
    if (!payeq) return;
    persons_vec_free(payeq->persons);
    payment_record_vec_free(payeq->payment_records);

    free(payeq);
}

void payeq_calc_avg_payment(struct payeq *payeq) {
    float total = 0.0;
    for (size_t i = 0; i < payeq->persons->size; i++) {
        total += payeq->persons->list[i]->paid;
    }
    payeq->avg_payment = total / payeq->persons->size;
}

void payeq_calc_debts_and_credits(struct payeq *payeq) {
    float debt         = 0;
    payeq->total_debts = 0;
    for (size_t i = 0; i < payeq->persons->size; i++) {
        struct person *p = payeq->persons->list[i];
        debt             = 0;
        if (p->paid < payeq->avg_payment) {
            debt    = payeq->avg_payment - p->paid;
            p->debt = debt;
            payeq->total_debts += debt;
        } else {
            p->credit = p->paid - payeq->avg_payment;
        }
    }
}

void payeq_bring_equity(struct payeq *payeq) {
    float td;          // Copy of total debts
    float tm;          // Transfered money
    struct person *gd; // Greatest debtor
    struct person *gc; // Greatest creditor

    td = payeq->total_debts;
    gd = gc = NULL;

    while (td > 0) {

        // Find max debtor and creditor
        float max_c = 0;
        float max_d = 0;
        for (size_t i = 0; i < payeq->persons->size; i++) {
            struct person *p = payeq->persons->list[i];
            if (p->debt > max_d) {
                max_d = p->debt;
                gd    = p;
            } else if (p->credit > max_c) {
                max_c = p->credit;
                gc    = p;
            }
        }

        // Find how much money transfer to creditor
        tm = 0;
        tm = gc->credit - gd->debt >= 0
                 ? gd->debt // Creditor can take all gd->debt
                 : gd->debt - (gd->debt -
                               gc->credit); // Creditor take a part of gd->debt

        // Save the payment
        struct payment_record *new_pr = payment_record_generate(gd, gc, tm);
        check_ptr(new_pr);
        payment_record_vec_push(payeq->payment_records, new_pr);

        // Update for next iteration
        gd->debt -= tm;
        gc->credit -= tm;
        td -= tm;
    }
}

void payeq_add_person(struct payeq *payeq, struct person *person) {
    person_vec_push(payeq->persons, person);
}

void payeq_process(struct payeq *payeq) {
    payeq_calc_avg_payment(payeq);
    payeq_calc_debts_and_credits(payeq);
    payeq_bring_equity(payeq);
}

void print_menu(void) {
    printf("\n--- Payeq CLI ---\n");
    printf("1. Add Person\n");
    printf("2. Process Expense Sharing\n");
    printf("3. Display Payments\n");
    printf("4. Quit\n");
}

float get_valid_float_input(const char *prompt) {
    float value;
    char buffer[256];
    int valid_input;

    do {
        valid_input = 1;
        printf("%s", prompt);
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("[!] Error reading input. Please try again.\n");
            valid_input = 0;
            continue;
        }

        if (sscanf(buffer, "%f", &value) != 1) {
            printf("[!] Invalid input. Please enter a valid number.\n");
            valid_input = 0;
        }

        // Clear the input buffer
        while (getchar() != '\n') {
        }

    } while (!valid_input);

    return value;
}

int main(void) {
    struct payeq *payeq = payeq_alloc();
    check_ptr(payeq);

    int choice = 4;
    do {
        print_menu();
        printf("Choose an option: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
        case 1: {
            char name[PERSON_MAX_NAME_SIZE];
            float paid;
            printf("Enter person's name: ");
            fgets(name, sizeof(name), stdin);
            name[strcspn(name, "\n")] = '\0';
            printf("Enter amount paid by %s: $", name);
            scanf("%f", &paid);
            getchar();

            struct person *new_person = person_generate(name, paid);
            check_ptr(new_person);
            payeq_add_person(payeq, new_person);
            printf("\n[*] %s added successfully.\n", name);
            break;
        }
        case 2:
            payeq_process(payeq);
            printf("\n[*] Expense sharing processed successfully.\n");
            break;
        case 3:
            for (size_t i = 0; i < payeq->payment_records->size; i++) {
                struct payment_record *pr = payeq->payment_records->list[i];
                printf("\n[*] %s pays $%.2f to %s\n", pr->from->name, pr->quantity,
                       pr->to->name);
            }
            break;
        case 4:
            printf("\n[*] Quitting.\n");
            break;
        default:
            printf("Invalid choice. Please try again.\n");
        }
    } while (choice != 4);

    payeq_free(payeq);

    return 0;
}
