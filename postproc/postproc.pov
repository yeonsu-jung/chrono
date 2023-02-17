#include "colors.inc"    // The include files contain
#include "functions.inc"
#include "math.inc"
#include "transforms.inc"

background { color White }
light_source { <0, 0, 600> color White}
// light_source { <5, 2.5, 5> color White}

camera {
    // location <150.9944, -308.0409, -699.8654>
    // location <-414.0391,  423.2913,  506.8492> // worked for spheres
    // location <0, 2500,14000 - 30*clock> // Movie 1
    //location <0, 2500,14000>
    //location <0, 750, 750>
    location 1.5*<0,  550,550>
    look_at  <0, 0, 0>
    angle 10
    }

#declare RodDiameter = 3;
// #declare LayerID = "All";
// #declare NumPoints = 400;
// #declare ColorFile = concat("Data/rod_color.txt");

#declare Iterator = 0;
#declare DataFolder = "data";
#declare ColorFile = concat(DataFolder,"/color_order.txt");
// #declare ContactFile = concat(DataFolder,"/contacts.txt");

#declare Iterator = 0;
#fopen Clr ColorFile read   //load shape data
#while (defined(Clr))
    #read (Clr,X,Y,Z)
    #declare Iterator = Iterator + 1;
#end
#fclose Clr

#declare NumColors = Iterator;
#declare ColorOrder = array[NumColors];

#declare Iterator = 0;
#fopen Clr ColorFile read   //load shape data
#while (defined(Clr))
    #read (Clr,X,Y,Z)
    #declare ColorOrder[Iterator] = <X,Y,Z>;
    #declare Iterator = Iterator + 1;
#end
#fclose Clr


// #declare Iterator = 0;
// #fopen Con ContactFile read   //load shape data
// #while (defined(Con))
//     #read (Con,X,Y,Z)
//     #declare Iterator = Iterator + 1;
// #end
// #fclose Con

// #declare NumContacts = Iterator;
// #declare Contacts = array[NumContacts];

// #declare Iterator = 0;
// #fopen Con ContactFile read   //load shape data
// #while (defined(Con))
//     #read (Con,X,Y,Z)
//     #declare Contacts[Iterator] = <X,Y,Z>;
//     #declare Iterator = Iterator + 1;
// #end
// #fclose Con

#declare Index = 0;
#declare File = concat(DataFolder,"/rod_",str(Index,-4,0),".txt");
#while (file_exists(File))
    #declare Index = Index + 1;
    #declare File = concat(DataFolder,"/rod_",str(Index,-4,0),".txt");
#end
#declare NumRods = Index;
#debug concat("Number of rods: ",str(NumRods,5,0))

// Contacts
// #declare Iterator = 0;
// #while(Iterator < NumContacts)
//     #declare cen = Contacts[Iterator];    
//     sphere
//     {
//         cen,1
//         material
//             {
//                 texture { pigment{ color rgbt<0,0,0,0.2>}
//                         finish { phong 0.5 }
//                         } // end of texture
//                 interior{
//                         caustics 1
//                         } // end of interior
//             } // end of material
//     }
//     #declare Iterator = Iterator + 1;
// #end

#debug str(NumRods,5,0)

// Rods
#declare Iterator = 0;
#while(Iterator < NumRods)
    // #declare File = concat("Data/rod_",str(Iterator,-4,0),".txt")
    #declare File = concat(DataFolder,"/rod_",str(Iterator,-4,0),".txt")

    #if(file_exists(File))
        #fopen Rods File read   //load shape data
        #declare Jterator = 0;
        #while (defined(Rods))
            #read (Rods,X,Y,Z)
            #declare Jterator = Jterator + 1;
        #end
        #fclose Rods
    #end
    #declare NumPoints = Jterator;
    #declare Positions = array[NumPoints];

    #if(file_exists(File))
        #fopen Rods File read   //load shape data
        #declare Jterator = 0;
        #while (defined(Rods))
            #read (Rods,X,Y,Z)
            #declare Positions[Jterator] = <X,Y,Z>;
            #declare Jterator = Jterator + 1;
        #end        
        #fclose Rods
    #end

    // sphere_sweep {
    //     cubic_spline,
    //     NumPoints,
    //     #declare ticker = 0;

    //     #while (ticker < NumPoints)
    //         #declare P1 = Positions[ticker];
    //         P1, RodDiameter
    //         #declare ticker = ticker + 1;            
    //     #end    

    //     #declare RGB = ColorOrder[Iterator];
        
    //     texture {  pigment{color rgb RGB filter 0.5}
    //                             finish {diffuse 0.8
    //                                     specular 0.4                                    
    //                                     roughness 0.2
    //                                     }
    //                             } 
    //     }

      cylinder {
            Positions[0],     // Center of one end
            Positions[1],     // Center of other end
            RodDiameter            // Radius
            open           // Remove end caps
            #declare RGB = ColorOrder[Iterator];
        
            texture {  pigment{color rgb RGB filter 0.5}
                                    finish {diffuse 0.8
                                            specular 0.4                                    
                                            roughness 0.2
                                            }
                                    } 
            }
      
    // sphere
    // {
    //     <0,0,0>, 36
    //     material{
    //         texture { pigment{ color rgbt<1,1,1,0.9>}
    //                 finish { phong 1 }
    //                 } // end of texture
    //         interior{
    //                 ior 1
    //                 } // end of interior
    //         } // end of material
    // }

    #declare Iterator = Iterator + 1;
    
#end

   
        

        


// #fopen RF RodFiles read   //load shape data


// #while (defined(CTR))
//     #read (CTR, X, Y, Z)
//     #declare Positions[Index] = <X,Y,Z>;
//     #declare Index = Index + 1;
// #end       
// #fclose CTR