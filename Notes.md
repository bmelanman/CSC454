# OS2 Notes

## Serial I/O

### Serial Input

FIFO Buffer of input characters:

- Consumer reads from FIFO as needed/able
- Producer writes to buffer from an ISR

### Serial Output

FIFO buffer of output characters

- Producer writes to FIFO as needed/able
- Consumer reads from FIFO from an ISR

### Configuration

- Use 8 bits of data, no parity bit, 1 stop bit (a.k.a. '8N1')
- Enable serial IO interrupt(s)
- Unmask UART on the PIC

## Interrupt Troubleshooting

- Check IDT entries are correct in memory