# [Converting decimal to octal](https://codereview.stackexchange.com/questions/206499)
### tags: ['c++', 'beginner', 'algorithm', 'number-systems']

This is a simple program converting user input decimal numbers into octal ones. 

   

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