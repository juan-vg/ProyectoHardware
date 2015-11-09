//------------------------------------------------------------------------------------------------ 
//Function Name: __c_copy
// This function copies 10 words from one address to another
// orig: pointer to the source block 
// dest: pointer to the destination block
//------------------------------------------------------------------------------------------------ 
void __c_copy_10(int *orig, int *dest)
{ 
 int i; 
 for(i=0;i<10;i++)
 { 
      	dest[i]=orig[i];
    } 
} 
 
 
