#include <common.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/immap_85xx.h>
#include <ioports.h>
#include <spd.h>

#if defined(CFG_DRAM_TEST)

extern void serial_puts(const char *s);
extern void serial_putc(const char c);


#define MemoryTestBlockSize  0x00000004
#define MemoryTestDlsplaySize  0x00001000
#define CFG_MEMTEST_PAT   9


void showhexnumber(unsigned int num)
{
    int i;
    unsigned int temp1;
  
    serial_putc('0');
    serial_putc('x');
    for (i=28; i>=0; i-=4)
    {
        temp1 = (num>>i)&0xf;
        if (temp1<0xa)
            serial_putc(0x30+temp1);
        else
            serial_putc(0x37+temp1);
    }
}

#define TestDataBlock 0x00100000

void DataLineTest(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{

   u64 pattern[CFG_MEMTEST_PAT] = { 
                0x0000000000000000LL, 
                0xaaaaaaaaaaaaaaaaLL, 
                0x5555555555555555LL, 
                0xaaaaaaaaaaaaaaaaLL, 
                0x5555555555555555LL, 
                0x0000000000000000LL, 
                0xffffffffffffffffLL, 
                0x0000000000000000LL, 
                0xffffffffffffffffLL 
                } ; 


    unsigned int testaddress, testdata;
    unsigned int temp,i;
    unsigned int *addr;


    for (testaddress = MemoryTestStart; testaddress <(MemoryTestEnd-sizeof(unsigned int)); testaddress+=MemoryTestBlockSize)
    {


      if ((testaddress%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber(testaddress);
            serial_puts(" \r");
        }

	for (i=0; i<CFG_MEMTEST_PAT;i++)
	{

            testdata = pattern[i];;

	    addr = (unsigned int *)testaddress;
            *addr = testdata;

/*
            serial_puts("addr:");
            showhexnumber(addr);
            serial_puts(" \r\n");

            serial_puts("testdata:");
            showhexnumber(testdata);
            serial_puts(" \r\n");
*/
            addr++;
            *addr = (~testdata);
            addr--;
            temp = *addr;

            if (temp != testdata)
            {
                serial_puts("\nT:");
                showhexnumber(testaddress);
                serial_puts(" D1:");
                showhexnumber(testdata);
                serial_puts(" D2:");
                showhexnumber(~testdata);
                serial_puts(" R:");
                showhexnumber(temp);
                serial_puts(" ER\n");
                while(1);
            }

	}

    }
}

void AddressLineTest(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{
    unsigned int testaddress;
    unsigned int xor;
    unsigned int i;
    unsigned int *addr1;
    unsigned int *addr2;

    for (testaddress = MemoryTestStart; testaddress <MemoryTestEnd; testaddress+=MemoryTestBlockSize)
    {
        if ((testaddress%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber(testaddress);
            serial_puts(" \r");
        }

        addr1 = (unsigned int *)testaddress;
        for(xor = sizeof(unsigned int); xor > 0; xor <<= 1)
        {
            i = (testaddress ^ xor);
 
            if ((i>=MemoryTestStart)&&(i<=MemoryTestEnd))
            {
                addr2 = (unsigned int *)i;
                *addr1 = ~(*addr2);

                if (*addr1 == *addr2)
                {
                    serial_puts("\nT1:");
                    showhexnumber((unsigned int)addr1);
                    serial_puts(" T2:");
                    showhexnumber((unsigned int)addr2);
                    serial_puts(" R1:");
                    showhexnumber(*addr1);
                    serial_puts(" R2:");
                    showhexnumber(*addr2);
                    serial_puts(" ER\n");
                    while(1);
                }
            }
        }
    }
}

void MemoryWriteReadTest(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{
    unsigned int i=0, j=0;
    unsigned int *addr;
    /* WriteReadTest 1 */

    i= 0x5555aaaa;

    serial_puts("Writing...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;
        *addr = i;
        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" \r");
        }
    }

    serial_puts("\nReading...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;

        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != i)
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }

    serial_puts("\n");

    i= 0x00000001;

    serial_puts("Writing...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;
        *addr = i;
        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" \r");
        }
    }

    serial_puts("\nReading...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;

        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != i)
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }

    serial_puts("\n");

    i= 0x55555555;

    serial_puts("Writing...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;
        *addr = i;
        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" \r");
        }
    }

    serial_puts("\nReading...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;

        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != i)
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }

    serial_puts("\n");

    i= 0xaaaaaaaa;

    serial_puts("Writing...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;
        *addr = i;
        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" \r");
        }
    }

    serial_puts("\nReading...\n");

    for (j = MemoryTestStart; j <MemoryTestEnd; j+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)j;

        if ((j%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != i)
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }

    serial_puts("\n");

    /* WriteReadTest 2 */
    serial_puts("Writing...\n");

    for (i = MemoryTestStart; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
         addr = (unsigned int *)i;
         *addr = (unsigned int)(1 << (i % 32));

         if ((i%MemoryTestDlsplaySize)==0)
         {
             serial_puts("T:");
             showhexnumber((unsigned int)addr);
             serial_puts(" W:");
             showhexnumber((unsigned int)(1 << (i % 32)));
             serial_puts(" \r");
         }
    }

    serial_puts("\nReading...\n");

    for (i = MemoryTestStart; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)i;

        if ((i%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != (unsigned int)(1 << (i % 32)))
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber((unsigned int)(1 << (i % 32)));
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }

    serial_puts("\n");

    /* WriteReadTest 3 */
    serial_puts("Writing...\n");

    for (i = MemoryTestStart; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)i;
        *addr = (unsigned int)(i);

        if ((i%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" \r");
        }
    }

    serial_puts("\nReading...\n");

    for (i = MemoryTestStart; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)i;

        if ((i%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != (unsigned int)(i))
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber(i);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }

    serial_puts("\n");

    /* WriteReadTest 4 */
    serial_puts("Writing...\n");

    for (i = MemoryTestStart; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)i;
        *addr = (unsigned int)(~i);

        if ((i%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber((unsigned int)(~i));
            serial_puts(" \r");
        }
    }

    serial_puts("\nReading...\n");

    for (i = MemoryTestStart; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)i;

        if ((i%MemoryTestDlsplaySize)==0)
        {
            serial_puts("T:");
            showhexnumber((unsigned int)addr);
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" \r");
        }

        if (*addr != (unsigned int)(~i))
        {
            serial_puts("\nT:");
            showhexnumber((unsigned int)addr);
            serial_puts(" W:");
            showhexnumber((unsigned int)(~i));
            serial_puts(" R:");
            showhexnumber(*addr);
            serial_puts(" ER\n");
        }
    }
}

void BWL_Test(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{
    unsigned char BWL_pattern[8] = {
    0x12,
    0x34,
    0x56,
    0x78,
    0x9a,
    0xbc,
    0xde,
    0xf0};

    unsigned int i=0, j=0;
    unsigned int *addr;
    unsigned int num_patterns = 8;
    int result = 1;

    for (i = MemoryTestStart; i <(MemoryTestEnd-sizeof(unsigned int)*8); i+=MemoryTestBlockSize)
    {
        /* Write Byte-Word-Long_pattern by byte */
        addr = (unsigned int *)i;
        for (j = 0 ; j < num_patterns ; j++)
        {
            *addr = BWL_pattern[j];
            addr++;
        }

        /* check by byte */
        addr = (unsigned int *)i;
        for (j = 0 ; j < num_patterns ; j++)
        {
            if ( (unsigned int)*addr != BWL_pattern[j] )
                result = 0;
            addr++;
        }

        /* Write Byte-Word-Long_pattern by word */
        addr = (unsigned int *)i;
        for (j = 0 ; j < num_patterns ; j+=2)
        {
            *addr = (BWL_pattern[j]<<8)|(BWL_pattern[j+1]);
            addr++;
        }

        /* check by word */
        addr = (unsigned int *)i;
        for (j = 0 ; j < num_patterns ; j+=2)
        {
            if ( *addr != ((BWL_pattern[j]<<8)|(BWL_pattern[j+1])) )
                result = 0;
            addr++;
        }

        /* Write Byte-Word-Long_pattern by Long */
        addr = (unsigned int *)i;
        for (j = 0 ; j < num_patterns ; j+=4)
        {
            *addr = (BWL_pattern[j]<<24)|(BWL_pattern[j+1]<<16)|(BWL_pattern[j+2]<<8)|(BWL_pattern[j+3]);
            addr++;
        }

        /* check by Long */
        addr = (unsigned int *)i;
        for (j = 0 ; j < num_patterns ; j+=4)
        {
            if ( *addr != ((BWL_pattern[j]<<24)|(BWL_pattern[j+1]<<16)|(BWL_pattern[j+2]<<8)|(BWL_pattern[j+3])) )
                result = 0;
            addr++;
        }
        if (result==1)
        {
            if ((i%MemoryTestDlsplaySize)==0)
            {
                serial_puts("T:");
                showhexnumber(i);
                serial_puts(" Byte-Word-Long Test OK\r");
            }
        }
        else
        {
            serial_puts("T:");
            showhexnumber(i);
            serial_puts(" Byte-Word-Long Test ER\n");
        }

    }
}

void PO2_Test(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{
    unsigned int i;
    unsigned int *addr;
    int result = 1;
    for (i = 1; (MemoryTestStart + i) <MemoryTestEnd; i<<=1)
    {
        addr = (unsigned int *)(MemoryTestStart + i);
        *addr = 0;
    }

    for (i = 1; (MemoryTestStart + i) <MemoryTestEnd; i<<=1)
    {
        addr = (unsigned int *)(MemoryTestStart + i);
        serial_puts("T:");
        showhexnumber(i);
        serial_puts("Power of 2 Test OK ");
        if (*addr != 0)
        {
            serial_puts("ER\n");
            result = 0;
        }
        else
        {
            serial_puts("OK\r");
        }
    }

    /* Set power of 2 address to 0xffffffff */
    for (i = 1; (MemoryTestStart + i) <MemoryTestEnd; i<<=1)
    {
        addr = (unsigned int *)(MemoryTestStart + i);
        *addr = 0xffffffff;
    }

    /* Check the power of 2 address is 0 */
    for (i = 1; (MemoryTestStart + i) <MemoryTestEnd; i<<=1)
    {
        addr = (unsigned int *)(MemoryTestStart + i);
        serial_puts("T:");
        showhexnumber(i);
        serial_puts("Power of 2 Test OK ");
        if (*addr != 0xffffffff)
        {
            serial_puts("ER\n");
            result = 0;
        }
        else
        {
            serial_puts("OK\r");
        }
    }
    /*
    if (result == 0)
    {
        serial_puts('O');
        serial_puts('K');
        serial_puts('\r');
    }
    */
}

void WALKING10_Test(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{
    unsigned int i, j;
    unsigned int *addr;
    int result = 1;
	
    serial_puts("WALKING10 Test:Writing...\n");

    /* Set address to 1 and 1's complement */
    for (i = MemoryTestStart, j = 0; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {

            if ((i%MemoryTestDlsplaySize)==0)
            {
	            serial_puts("T:");
        	    showhexnumber(i);
	            serial_puts(" \r");
            }
	
        addr = (unsigned int *)i;
        *addr = ((j%2)==0)?1:(~1);
        j++;
    }
    serial_putc('\r');

    serial_puts("Writing...OK\n");

    /* read address to 1 and 1's complement */
    for (i = MemoryTestStart, j = 0; i <MemoryTestEnd; i+=MemoryTestBlockSize)
    {
        addr = (unsigned int *)i;
        if ((j%2) == 0)
        {
            if (*addr != 1)
                result = 0;
        }
        else
        {
            if (*addr != (~1))
                result = 0;
        }
        j++;
        if (result == 0)
        {
            serial_puts("\nT:");
            showhexnumber(i);
            serial_puts(" Walking 1 and 0 Test ER\n");
        }
        else
        {
            if ((i%MemoryTestDlsplaySize)==0)
            {
                serial_puts("T:");
                showhexnumber(i);
                serial_puts(" Walking 1 and 0 Test OK\r");
            }
        }
    }
}

void XOR_Test(unsigned int MemoryTestStart, unsigned int MemoryTestEnd)
{
    /*
    unsigned int XOR_pattern[] = {
            0x6DB6DB6D,
            0xB6DB6DB6,
            0xDB6DB6DB};
    */
    unsigned int i, j;
    unsigned int *addr;
    unsigned int checkvalue;
    unsigned int num_patterns = 3;
    int result = 1;

    serial_puts("XOR Test:Writing...\n");

    /* Write the XOR pattern */ /* 3 unsigned int = + 12 address*/
    for (i = MemoryTestStart; i < (MemoryTestEnd-sizeof(unsigned int)*num_patterns); i+=sizeof(unsigned int)*num_patterns)
    {
        addr = (unsigned int *)i;
        for ( j = 0; j < num_patterns; j++)
        {

            if ((i%MemoryTestDlsplaySize)==0)
            {
	            serial_puts("T:");
        	    showhexnumber(i);
	            serial_puts(" \r");
            }
		
            switch(j)
            {
                case 0:
                    *addr = 0x6DB6DB6D;
                break;
                case 1:
                    *addr = 0xB6DB6DB6;
                break;
                case 2:
                    *addr = 0xDB6DB6DB;
                break;
            }
            /* *addr = XOR_pattern[j]; */
            addr++;
        }
    }
    serial_putc('\r');
    /*
    for (i = 0x10000; i < 0x11000; i++)
    {
            addr = (unsigned int *)i;
            serial_puts('T');
            serial_puts(':');
            showhexnumber(i);
            serial_puts(' ');
            showhexnumber(*addr);
            serial_puts('\r'); serial_puts('\n');
    }
    */
    /* check XOR */
    for (i = MemoryTestStart; i < (MemoryTestEnd-sizeof(unsigned int)*num_patterns); i+=sizeof(unsigned int)*num_patterns)
    {
        addr = (unsigned int *)i;
        checkvalue = *addr;
        addr++;
        *addr = *addr^checkvalue;
        checkvalue = *addr;
        addr++;
        *addr = *addr^checkvalue;
        checkvalue = *addr;

        if((*addr^checkvalue) != 0)
        {
            serial_puts("\nT:");
            showhexnumber(i);
            serial_puts(" XOR Test ER ");
            showhexnumber(*addr^checkvalue);
            serial_puts("\n");
            result = 0;
        }
        else
        {
            if ((i%MemoryTestDlsplaySize)==0)
            {
                serial_puts("T:");
                showhexnumber(i);
                serial_puts(" XOR Test OK\r");
            }
        }
    }
}

#endif

