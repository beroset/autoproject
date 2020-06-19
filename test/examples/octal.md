This is simple program converts user inputted decimal numbers into octal ones. General advice is appreciated!

   

     #include <iostream>
    using namespace std;
    main()
    {
    	int de,oc,y,i=1,octal;
    	float decimal,deci,x;
    	cout<<"Enter decimal no :: ";
    	cin>>decimal;
    	de=decimal;
    	deci=decimal-de;
    	cout<<"("<<decimal<<")10 = (";
    	while(de>0)
    	{
    		oc=de%8;
    		de=de/8;
    		octal=octal+(oc*i);
    		i=i*10;
    	}cout<<octal<<".";
    	while(deci>0)
    	{
    		x=deci*8;
    		y=x;
    		deci=x-y;
    		cout<<y;
    	}
    	cout<<")8";
    }