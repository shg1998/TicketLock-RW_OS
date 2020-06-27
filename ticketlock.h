// Ticket lock.
struct ticketlock {
    int turn;
    int total;

    // For debugging:
    char *name;
};

