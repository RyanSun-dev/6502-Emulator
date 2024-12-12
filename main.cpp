#include <stdio.h>

using Byte = unsigned char;
using Word = unsigned short;

using u32 = unsigned int;
struct Mem
{
    static constexpr u32 MAX_MEM = 1024 * 64;
    Byte Data[MAX_MEM];

    void Initialize()
    {
        for(u32 i = 0;i < MAX_MEM; i++)
        {
            Data[i] = 0;
        }
    }

    // read 1 byte
    Byte operator[](u32 Address) const
    {
        return Data[Address];
    }

    // write 1 byte
    Byte& operator[](u32 Address)
    {
        return Data[Address];
    }

    // write 2 bytes
    void WriteWord(u32& Cycles, Word Value, u32 Address)
    {
        Data[Address] = Value & 0xFF;
        Cycles--;
        Data[Address + 1] = (Value >> 8);
        Cycles--;
    }
};


struct CPU
{
   

    Word PC;    //Program Counter
    Word SP;    //Stack Pointer

    Byte A, X, Y;   //Registers


    Byte C : 1; //Carry Flag
    Byte Z : 1; //Zero Flag   
    Byte I : 1; //Interrupt Disable
    Byte D : 1; //Decimal Mode
    Byte B : 1; //Break Command
    Byte V : 1; //Overflow Command
    Byte N : 1; //Negative Flag

    void Reset(Mem& memory){
        PC = 0xFFFC;
        SP = 0x0100;
        C = Z = I = D = B = V = N = 0;
        A = X = Y = 0;
        memory.Initialize();

    }

    //opcodes
    static constexpr Byte 
    INS_LDA_IM = 0xA9,
    INS_LDA_ZP = 0xA5,
    INS_LDA_ZPX = 0xB5,
    INS_JSR = 0x20;

    Byte FetchByte(u32& Cycles, Mem& memory)
    {
        Byte Data = memory[PC];
        PC++;
        Cycles--;
        return Data;
    }

    Byte ReadByte(u32& Cycles, Byte Address, Mem& memory)
    {
        Byte Data = memory[Address];
        Cycles--;
        return Data;
    }

    Word FetchWord(u32& Cycles, Mem& memory)
    {
        // 6502 is little endian
        Word Data = memory[PC];
        PC++;
        Cycles--;

        Data |= (memory[PC] << 8);
        PC++;
        Cycles--;
        return Data;
    }

    void LDASetStatus()
    {
        Z  = (A==0);
         N = (A & 0b10000000) > 0;
    }

    void Execute(u32 Cycles,Mem& memory)
    {
        while(Cycles > 0)
        {
            Byte Ins = FetchByte(Cycles,memory);
            switch (Ins)
            {
            case INS_LDA_IM:
            {
                Byte value = FetchByte(Cycles, memory);
                A = value;
                LDASetStatus();
            }break;
            case INS_LDA_ZP:
            {
                Byte ZeroPageAddress = FetchByte(Cycles, memory); 
                A = ReadByte(Cycles,ZeroPageAddress, memory);
                LDASetStatus();
            }break;
            case INS_LDA_ZPX:
            {
                Byte ZeroPageAddress = FetchByte(Cycles, memory);
                ZeroPageAddress += X;
                Cycles--;
                A = ReadByte(Cycles,ZeroPageAddress, memory);
                LDASetStatus();
            }break;
            case INS_JSR:
            {
                Word SubAddress = FetchWord(Cycles,memory);
                memory.WriteWord(Cycles,PC-1,SP);
                SP++;
                PC = SubAddress;
                Cycles--;

            }break;
            default:
            {
                printf("Instruction did not execute %d", Ins);
            }
                break;
            }
        }
    }
};

int main(){
    Mem mem;
    CPU cpu;
    cpu.Reset(mem);
    // start - inline a program
    mem[0xFFFC] = CPU::INS_JSR;
    mem[0xFFFD] = 0x42;
    mem[0xFFFE] = 0x42;
    mem[0x4242] = CPU::INS_LDA_IM;
    mem[0x4243] = 0x84;
    // end - inline a program
    cpu.Execute(9,mem);
    return 0;
}