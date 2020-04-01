#include <stdio.h>
#include <stdlib.h>

union Numar
{
    unsigned int x;
    unsigned char b[4];

} vir,vic,rni;

struct detectii
{
    float prag;
    int cifra,x,y,L,H;
    unsigned char *culoare;
    FILE *f;
};

unsigned int xorshift32(unsigned int x)
{
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

void liniarizare(char *fisier_sursa,unsigned int **P,unsigned int *L,unsigned int *H)
{
    FILE *fin=fopen(fisier_sursa,"rb");
    int i;
    if(fin==NULL)
    {
        printf("Fisier inexistent\n");
        exit(0);
    }
    fseek(fin,18,SEEK_SET);
    fread(&(*L),4,1,fin);
    fread(&(*H),4,1,fin);
    unsigned int n;
    n=*L**H;
    *P=(int *) malloc(sizeof(int)*n);
    if(*P==NULL)
    {
        printf("Nu poate fi alocata memorie pentru liniarizare\n");
        exit(0);
    }
    fseek(fin,54,SEEK_SET);
    for(i=0; i<n; i++)
    {
        fread(&((*P)[i]),3,1,fin);
    }

    fclose(fin);
}

void salvare(char *imagine,char *fisier_sursa,unsigned int *P,unsigned int L,unsigned int H)
{
    FILE *fout=fopen(imagine,"wb");
    FILE *fin=fopen(fisier_sursa,"rb");
    if(fin==NULL)
    {
        printf("Nu s-a putut deschide fisierul pentru copierea header-ului");
        exit(0);
    }

    if(fout==NULL)
    {
        printf("Nu s-a putut crea fisierul pentru salvarea imaginii");
        exit(0);
    }
    int n=L*H;
    int i=0;
    unsigned char c;
    fseek(fin,0,SEEK_SET);
    while(fread(&c,1,1,fin)==1 && i<54)
    {
        fwrite(&c,1,1,fout);
        // fflush(fout);
        i++;
    }
    fseek(fout,54,SEEK_SET);
    for(i=0; i<n; i++)
        fwrite(&P[i],3,1,fout);
    fclose(fout);
    fclose(fin);
}

void criptare(char *fisier_sursa,char * fisier_criptat,char *cheie)
{
    unsigned int L=0,H=0,i;
    unsigned int *P,*R;
    unsigned int key;
    liniarizare(fisier_sursa,&P,&L,&H);
    FILE *secret_key=fopen(cheie,"r");
    unsigned int n=L*H;
    unsigned int m=2*L*H;
    R=(unsigned int *) malloc(sizeof(unsigned int)*m);
    if(R==NULL)
    {
        printf("Nu s-a putut aloca spatiu vectorului generat\n");
        exit(0);
    }
    fscanf(secret_key,"%u",&R[0]);
    fscanf(secret_key,"%u",&key);
    for(i=1; i<m; i++)
    {
        R[i]=xorshift32(R[i-1]);
    }
    for(i=n-1; i>=1; i--)
    {
        unsigned int aux;
        aux=P[R[(R[n-i])%n]%i];
        P[R[(R[n-i])%n]%i]=P[i];
        P[i]=aux;
    }
    vir.x=key;
    vic.x=P[0];
    rni.x=R[n];
    vic.b[0]=vic.b[0]^vir.b[0]^rni.b[0];
    vic.b[1]=vic.b[1]^vir.b[1]^rni.b[1];
    vic.b[2]=vic.b[2]^vir.b[2]^rni.b[2];
    P[0]=vic.x;
    for(i=1; i<n; i++)
    {
        vir.x=P[i-1];
        vic.x=P[i];
        rni.x=R[n+i];
        vic.b[0]=vic.b[0]^vir.b[0]^rni.b[0];
        vic.b[1]=vic.b[1]^vir.b[1]^rni.b[1];
        vic.b[2]=vic.b[2]^vir.b[2]^rni.b[2];
        P[i]=vic.x;
    }

    salvare(fisier_criptat,fisier_sursa,P,L,H);
    fclose(secret_key);
    free(P);
}


void decriptare(char *fisier_sursa,char * fisier_criptat,char *fisier_decriptat,char *cheie)
{
    unsigned int L=0,H=0,i;
    unsigned int *P,*R;
    unsigned int key;
    liniarizare(fisier_criptat,&P,&L,&H);
    FILE *secret_key=fopen(cheie,"r");

    unsigned int n=L*H;
    unsigned int m=2*L*H;
    R=(unsigned int *) malloc(sizeof(unsigned int)*m);
    if(R==NULL)
    {
        printf("Nu s-a putut aloca spatiu vectorului generat\n");
        exit(0);
    }
    fscanf(secret_key,"%u",&R[0]);
    fscanf(secret_key,"%u",&key);
    for(i=1; i<m; i++)
    {
        R[i]=xorshift32(R[i-1]);
    }

    for(i=n; i>=1; i--)
    {
        vir.x=P[i-1];
        vic.x=P[i];
        rni.x=R[n+i];
        vic.b[0]=vic.b[0]^vir.b[0]^rni.b[0];
        vic.b[1]=vic.b[1]^vir.b[1]^rni.b[1];
        vic.b[2]=vic.b[2]^vir.b[2]^rni.b[2];
        P[i]=vic.x;
    }
    vir.x=key;
    vic.x=P[0];
    rni.x=R[n];
    vic.b[0]=vic.b[0]^vir.b[0]^rni.b[0];
    vic.b[1]=vic.b[1]^vir.b[1]^rni.b[1];
    vic.b[2]=vic.b[2]^vir.b[2]^rni.b[2];
    P[0]=vic.x;
    for(i=1; i<n; i++)
    {
        unsigned int aux;
        aux=P[R[(R[n-i])%n]%i];
        P[R[(R[n-i])%n]%i]=P[i];
        P[i]=aux;
    }

    salvare(fisier_decriptat,fisier_sursa,P,L,H);
    fclose(secret_key);
    free(P);
}
void Chi_test(char *imagine)
{
    printf("Chi-squared test on RGB channels for %s:\n",imagine);
    unsigned int i,L,H;
    float SR=0.0,SB=0.0,SG=0.0,f;
    unsigned char x;
    FILE *fin=fopen(imagine,"rb");
    if(fin==NULL)
    {
        printf("Nu se poate deschide fisierul in cadrul functiei Chi_test, ori acesta este inexistent\n");
        exit(0);
    }
    unsigned int *R,*B,*G;
    R=(unsigned int*) calloc(256,sizeof(unsigned int));
    if(R==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru vectorul de frecventa corespunzator canalului rosu din functia Chi_test\n");
        exit(0);
    }
    G=(unsigned int*) calloc(256,sizeof(unsigned int));
    if(G==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru vectorul de frecventa corespunzator canalului verde din functia Chi_test\n");
        exit(0);
    }
    B=(unsigned int*) calloc(256,sizeof(unsigned int));
    if(B==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru vectorul de frecventa corespunzator canalului albastru din functia Chi_test\n");
        exit(0);
    }
    fseek(fin,18,SEEK_SET);
    fread(&L,4,1,fin);
    fread(&H,4,1,fin);
    while(fread(&x,1,1,fin)==1)
    {
        B[x]++;
        if(fread(&x,1,1,fin)==1)
            G[x]++;
        if(fread(&x,1,1,fin)==1)
            R[x]++;
    }
    f=(L*H)/256;
    for(i=0; i<256; i++)
    {
        SR=SR+(R[i]*R[i])/f-2*R[i]+f;
        SG=SG+(G[i]*G[i])/f-2*G[i]+f;
        SB=SB+(B[i]*B[i])/f-2*B[i]+f;
    }
    printf("Canal rosu: %.2f \n",SR);
    printf("Canal verde: %.2f \n",SG);
    printf("Canal albastru: %.2f \n",SB);
    printf("\n");
    free(R);
    free(G);
    free(B);
    fclose(fin);
}


int comp(const void *a,const void *b)
{
    struct detectii *elemA=a;
    struct detectii *elemB=b;
    if(elemA->prag==elemB->prag)
        return 0;
    if(elemA->prag<elemB->prag)
        return 1;
    else
        return -1;
}

void colorare(FILE *imagine,FILE *sursa,FILE * sablon,char *c,int x,int y)
{
    int i,j,k,l,t=x,u=y;
    unsigned int L_sursa,H_sursa,L_sablon,H_sablon;
    fseek(sursa,18,SEEK_SET);
    fread(&L_sursa,4,1,sursa);
    fread(&H_sursa,4,1,sursa);
    int padding_sursa;
    if(L_sursa % 4 != 0)
        padding_sursa = 4 - (3 * L_sursa) % 4;
    else
        padding_sursa = 0;
    fseek(sablon,18,SEEK_SET);
    fread(&L_sablon,4,1,sablon);
    fread(&H_sablon,4,1,sablon);
    fseek(imagine,54+x*(L_sursa*3+padding_sursa)+y*3,SEEK_SET);
    l=0;
    k=H_sablon-1;
    while(k>=0 && t>=0)
    {
        while(l!=L_sablon && u!=L_sursa)
        {
            if(l==0 || l==L_sablon-1 || k==H_sablon-1 ||k==0 )
            {
                fwrite(&(c[2]),1,1,imagine);
                fwrite(&(c[1]),1,1,imagine);
                fwrite(&(c[0]),1,1,imagine);
            }
            else
                fseek(imagine,3,SEEK_CUR);
            l++;
            u++;
        }
        if(t==0)
        {
            break;
        }
        else
        {
            k--;
            t--;
            l=0;
            u=y;
            fseek(imagine,54+t*(L_sursa*3+padding_sursa)+u*3,SEEK_SET);
        }
    }
}

void detectie(FILE *sursa,FILE *sablon,struct detectii **detect,unsigned int *dn,float ps,int cifra,unsigned char **culoare)
{
    unsigned char x,y,pRGB[3],auxc,**sablonf,**sursaf;
    int i,j,k,l,padding_sursa,padding_sablon;
    unsigned int L_sursa,H_sursa,L_sablon,H_sablon;
    float fI=0.0,S=0.0;
    fseek(sursa,18,SEEK_SET);
    fread(&L_sursa,4,1,sursa);
    fread(&H_sursa,4,1,sursa);
    if(L_sursa % 4 != 0)
        padding_sursa = 4 - (3 * L_sursa) % 4;
    else
        padding_sursa = 0;
    fseek(sablon,18,SEEK_SET);
    fread(&L_sablon,4,1,sablon);
    fread(&H_sablon,4,1,sablon);
    if(L_sablon % 4 != 0)
        padding_sablon = 4 - (3 * L_sablon) % 4;
    else
        padding_sablon= 0;
    sablonf=(unsigned char**)malloc(H_sablon*sizeof(unsigned char*));
    if(sablonf==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru matricea de pixeli a sablonului %d",cifra);
        exit(0);
    }
    for(i=0; i<H_sablon; i++)
    {
        sablonf[i]=(unsigned char*)malloc(L_sablon*sizeof(unsigned char));
        if(sablonf[i]==NULL)
        {
            printf("Nu s-a putut aloca memorie pentru vectorul %d al matricei de pixeli a sablonului %d",i,cifra);
            exit(0);
        }
    }
    fseek(sablon,54,SEEK_SET);
    for(i=0; i<H_sablon; i++)
    {
        for(j=0; j<L_sablon; j++)
        {
            fread(pRGB,3,1,sablon);
            auxc = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
            sablonf[i][j]=auxc;
            S=S+sablonf[i][j];
        }
        fseek(sablon,padding_sablon,SEEK_CUR);
    }
    fseek(sablon,0,SEEK_SET);
    sursaf=(unsigned char**)malloc(H_sursa*sizeof(unsigned char*));
    if(sursaf==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru matricea de pixeli a fisierului sursa la al %d apel al functiei",cifra+1);
        exit(0);
    }
    for(i=0; i<H_sursa; i++)
    {
        sursaf[i]=(unsigned char*)malloc(L_sursa*sizeof(unsigned char));
        if(sursaf[i]==NULL)
        {
            printf("Nu s-a putut aloca memorie pentru vectorul %d al matricei de pixeli  a fisierului sursa la al %d apel al functiei",i,cifra);
            exit(0);
        }
    }
    fseek(sursa,54,SEEK_SET);
    for(i=0; i<H_sursa; i++)
    {
        for(j=0; j<L_sursa; j++)
        {
            fread(pRGB,3,1,sursa);
            auxc = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
            sursaf[i][j]=auxc;
        }
        fseek(sursa,padding_sursa,SEEK_CUR);
    }
    fseek(sursa,0,SEEK_SET);
    S=S/(L_sablon*H_sablon);
    float sigmaS=0.0,sigmafI=0.0,corr=0.0;
    for(k=0; k<H_sablon; k++)
    {
        for(l=0; l<L_sablon; l++)
        {
            sigmaS=sigmaS+(sablonf[k][l]-S)*(sablonf[k][l]-S);
        }
    }
    sigmaS=sigmaS/(float)(L_sablon*H_sablon-1);
    sigmaS=sqrt((float)sigmaS);
    for(i=H_sablon-1; i<H_sursa; i++)
    {
        for(j=0; j<L_sursa; j++)
        {
            int t=i;
            k=H_sablon-1;
            l=0;
            int u=j;
            if(u+L_sablon>=L_sursa)
            {
                break;
            }
            while(k>=0 && t>=0)
            {
                while(l!=L_sablon && u!=L_sursa)
                {
                    fI=fI+(float)sursaf[t][u];
                    l++;
                    u++;
                }
                if(t==0)
                {
                    fI=fI/(float)(L_sablon*H_sablon);
                    break;
                }
                else
                {
                    k--;
                    t--;
                    l=0;
                    u=j;
                }
            }
            if(t!=0 )
                fI=fI/(float)(L_sablon*H_sablon);
            t=i;
            u=j;
            k=H_sablon-1;
            l=0;
            if(fI==0)
                continue;
            while(k>=0)
            {
                while(t>=0&& l<L_sablon && u<L_sursa)
                {
                    sigmafI=sigmafI+((float)sursaf[t][u]-fI)*((float)sursaf[t][u]-fI);
                    l++;
                    u++;
                }
                while(l<L_sablon)
                {
                    sigmafI=sigmafI+(0.0-fI)*(0.0-fI);
                    l++;
                }
                if(t==0)
                    break;
                if(t>0)
                {
                    t--;
                    u=j;
                }
                k--;
                l=0;
            }
            t=i;
            u=j;
            k=H_sablon-1;
            l=0;
            sigmafI=sigmafI/(float)(L_sablon*H_sablon-1);
            sigmafI=sqrt((float)sigmafI);
            if(sigmafI==0)
                continue;
            while(k>=0)
            {
                while(t>=0&& l<L_sablon && u<L_sursa)
                {
                    corr=corr+(((float)sursaf[t][u]-fI)*((float)sablonf[k][l]-S))*(1/(float)(sigmafI*sigmaS));
                    l++;
                    u++;
                }
                while(l<L_sablon)
                {
                    corr=corr+((0.0-fI)*((float)sablonf[k][l]-S))*(1/(float)(sigmafI*sigmaS));
                    l++;
                }
                if(t>0)
                {
                    t--;
                    u=j;
                }
                k--;
                l=0;
            }
            corr=corr/(float)(L_sablon*H_sablon);
            if(corr>=ps)
            {
                (*detect)[(*dn)-1].prag=corr;
                (*detect)[(*dn)-1].cifra=cifra;
                (*detect)[(*dn)-1].x=i;
                (*detect)[(*dn)-1].y=j;
                (*detect)[(*dn)-1].culoare=(char*)malloc(3*sizeof(char));
                if((*detect)[(*dn)-1].culoare==NULL)
                {
                    printf("Nu s-a putut aloca memorie pentru culoarea chenarului detectiei %d",(*dn)-1);
                    exit(0);
                }
                (*detect)[(*dn)-1].culoare[0]=culoare[cifra][0];
                (*detect)[(*dn)-1].culoare[1]=culoare[cifra][1];
                (*detect)[(*dn)-1].culoare[2]=culoare[cifra][2];
                (*detect)[(*dn)-1].f=sablon;
                (*detect)[(*dn)-1].L=L_sablon;
                (*detect)[(*dn)-1].H=H_sablon;
                struct detectii *aux;
                (*dn)++;
                aux=(struct detectii*)realloc(*detect,sizeof(struct detectii)*(*dn));
                if(aux==NULL)
                {
                    printf("Nu s-a putut aloca memorie pentru marirea dimensiunii vectorului de detectii");
                    exit(0);
                }
                *detect=aux;
            }
            corr=0.0;
            fI=0.0;
            sigmafI=0.0;
            t=i;
        }
    }
    free(*sablonf);
    free(sablonf);
    free(*sursaf);
    free(sursaf);
}

void elim_max(struct detectii **d,unsigned int *n)
{
    int i,j,k;
    for(i=*n-2; i>=0; i--)
    {
        unsigned int L_i,H_i;
        L_i=(*d)[i].L;
        H_i=(*d)[i].H;
        for(j=*n-1; j>i; j--)
        {
            unsigned int L_j,H_j;
            L_j=(*d)[j].L;
            H_j=(*d)[j].H;
            float suprapunere=0.0;
            if((*d)[i].x>=(*d)[j].x && (*d)[j].x>=((*d)[i].x-H_i))
            {
                if((*d)[i].y<=(*d)[j].y && (*d)[j].y<=(*d)[i].y+L_i)
                {
                    int intersectie=((*d)[j].x-((*d)[i].x-H_i))*((*d)[i].y+L_i-(*d)[j].y);
                    suprapunere=((float)intersectie)/((float)(L_i*H_i+L_j*H_j-intersectie));
                }
                if((*d)[j].y<=(*d)[i].y && (*d)[i].y<=(*d)[j].y+L_j)
                {
                    int intersectie=((*d)[j].x-((*d)[i].x-H_i))*((*d)[j].y+L_j-(*d)[i].y);
                    suprapunere=((float)intersectie)/((float)(L_i*H_i+L_j*H_j-intersectie));
                }
            }
            if((*d)[j].x>(*d)[i].x && (*d)[i].x>(*d)[j].x-H_j)
            {
                if((*d)[i].y<=(*d)[j].y && (*d)[j].y<=(*d)[i].y+L_i)
                {
                    int intersectie=((*d)[i].x-((*d)[j].x-H_j))*((*d)[i].y+L_i-(*d)[j].y);
                    suprapunere=((float)intersectie)/((float)(L_i*H_i+L_j*H_j-intersectie));
                }
                if((*d)[j].y<=(*d)[i].y && (*d)[i].y<=(*d)[j].y+L_j)
                {
                    int intersectie=((*d)[i].x-((*d)[j].x-H_j))*((*d)[j].y+L_j-(*d)[i].y);
                    suprapunere=((float)intersectie)/((float)(L_i*H_i+L_j*H_j-intersectie));
                }
            }
            if(suprapunere>0.2)
            {
                for(k=j; k<*n-1; k++)
                    (*d)[k]=(*d)[k+1];
                (*n)--;
                *d=(struct detectii*)realloc(*d,sizeof(struct detectii)*(*n));
            }
        }
    }
}

void citire(char **fisier,FILE *input)
{
    *fisier=(char *) malloc(sizeof(char)*101);
    if(*fisier==NULL)
    {
        printf("Nu poate fi alocata memorie pentru numele fisierului sursa");
        return 0;
    }
    fgets(*fisier,40,input);
    (*fisier)[strlen(*fisier)-1]='\0';
}



int main()
{
    //Modul criptare
    printf("Se incepe procesul de criptare si se decriptare a imaginii sursa...\n\n");
    FILE *input=fopen("in.txt","r");
    char *fisier_s,*fisier_criptat,*cheie,*fisier_decriptat;
    citire(&fisier_s,input);
    citire(&fisier_criptat,input);
    citire(&fisier_decriptat,input);
    citire(&cheie,input);
    criptare(fisier_s,fisier_criptat,cheie);
    decriptare(fisier_s,fisier_criptat,fisier_decriptat,cheie);
    printf("S-a criptat si decriptat imaginea...\n\n");
    printf("Se afiseaza rezultatele testului...\n\n");
    Chi_test(fisier_s);
    Chi_test(fisier_criptat);
    printf("\n\n");
    printf("Se incepe procesul de detectie a cifrelor in imagine\n\n");



    float ps=0.5;
    char *afis_f,*cif0,*cif1,*cif2,*cif3,*cif4,*cif5,*cif6,*cif7,*cif8,*cif9;
    citire(&afis_f,input);
    citire(&cif0,input);
    citire(&cif1,input);
    citire(&cif2,input);
    citire(&cif3,input);
    citire(&cif4,input);
    citire(&cif5,input);
    citire(&cif6,input);
    citire(&cif7,input);
    citire(&cif8,input);
    citire(&cif9,input);
    FILE *fisier_sursa=fopen(fisier_s,"rb+");
    if(fisier_sursa==NULL)
    {
        printf("Nu s-a putut deschide fisierul sursa");
        return 0;
    }
    FILE *afis_final=fopen(afis_f,"wb+");
    if(afis_final==NULL)
    {
        printf("Nu s-a putut crea imagine finala");
        return 0;
    }

    FILE *cifra0=fopen(cif0,"rb");
    if(cifra0==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 0");
        return 0;
    }
    FILE *cifra1=fopen(cif1,"rb");
    if(cifra1==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 1");
        return 0;
    }
    FILE *cifra2=fopen(cif2,"rb");
    if(cifra2==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 2");
        return 0;
    }
    FILE *cifra3=fopen(cif3,"rb");
    if(cifra3==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 3");
        return 0;
    }
    FILE *cifra4=fopen(cif4,"rb");
    if(cifra4==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 4");
        return 0;
    }
    FILE *cifra5=fopen(cif5,"rb");
    if(cifra5==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 5");
        return 0;
    }
    FILE *cifra6=fopen(cif6,"rb");
    if(cifra6==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 6");
        return 0;
    }
    FILE *cifra7=fopen(cif7,"rb");
    if(cifra7==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 7");
        return 0;
    }
    FILE *cifra8=fopen(cif8,"rb");
    if(cifra8==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 8");
        return 0;
    }
    FILE *cifra9=fopen(cif9,"rb");
    if(cifra9==NULL)
    {
        printf("Nu s-a putut deschide fisierul ce contine cifra 9");
        return 0;
    }
    struct detectii *d;
    unsigned int n=1;
    int i;
    unsigned char **culoare,x;
    culoare=(unsigned char**)malloc(sizeof(unsigned char*)*10);
    if(culoare==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru matricea de culori");
        return 0;
    }
    for(i=0; i<10; i++)
    {
        culoare[i]=(unsigned char*)malloc(sizeof(unsigned char)*3);
        if(culoare[i]==NULL)
        {
            printf("Nu s-a putut aloca memorie pentru linia %d a matricei de culori",i);
            return 0;
        }
    }
    culoare[0][0]=255;
    culoare[0][1]=0;
    culoare[0][2]=0;
    culoare[1][0]=255;
    culoare[1][1]=225;
    culoare[1][2]=0;
    culoare[2][0]=0;
    culoare[2][1]=225;
    culoare[2][2]=0;
    culoare[3][0]=0;
    culoare[3][1]=255;
    culoare[3][2]=255;
    culoare[4][0]=255;
    culoare[4][1]=0;
    culoare[4][2]=255;
    culoare[5][0]=0;
    culoare[5][1]=0;
    culoare[5][2]=255;
    culoare[6][0]=192;
    culoare[6][1]=192;
    culoare[6][2]=192;
    culoare[7][0]=255;
    culoare[7][1]=140;
    culoare[7][2]=0;
    culoare[8][0]=128;
    culoare[8][1]=0;
    culoare[8][2]=128;
    culoare[9][0]=128;
    culoare[9][1]=0;
    culoare[9][2]=0;
    d=(struct detectii*)malloc(sizeof(struct detectii)*n);
    if(d==NULL)
    {
        printf("Nu s-a putut aloca memorie pentru vectorul de detectii");
        return 0;
    }
    detectie(fisier_sursa,cifra0,&d,&n,ps,0,culoare);
    detectie(fisier_sursa,cifra1,&d,&n,ps,1,culoare);
    detectie(fisier_sursa,cifra2,&d,&n,ps,2,culoare);
    detectie(fisier_sursa,cifra3,&d,&n,ps,3,culoare);
    detectie(fisier_sursa,cifra4,&d,&n,ps,4,culoare);
    detectie(fisier_sursa,cifra5,&d,&n,ps,5,culoare);
    detectie(fisier_sursa,cifra6,&d,&n,ps,6,culoare);
    detectie(fisier_sursa,cifra7,&d,&n,ps,7,culoare);
    detectie(fisier_sursa,cifra8,&d,&n,ps,8,culoare);
    detectie(fisier_sursa,cifra9,&d,&n,ps,9,culoare);
    d=(struct detectii*)realloc(d,sizeof(struct detectii)*(n-1));
    n--;
    qsort(d,n-1,sizeof(struct detectii),comp);
    elim_max(&d,&n);
    fseek(fisier_sursa,0,SEEK_SET);
    while(fread(&x,1,1,fisier_sursa)==1 )
    {
        fwrite(&x,1,1,afis_final);
        i++;
    }
    fseek(fisier_sursa,0,SEEK_SET);
    fseek(afis_final,0,SEEK_SET);
    for(i=0; i<n; i++)
        colorare(afis_final,fisier_sursa,d[i].f,d[i].culoare,d[i].x,d[i].y);
    printf("S-a terminat procesul de detectie a cifrelor\n");
    free(*culoare);
    free(culoare);
    free(d);
    free(fisier_s);
    free(fisier_decriptat);
    free(fisier_criptat);
    free(cheie);
    free(afis_f);
    free(cif0);
    free(cif1);
    free(cif2);
    free(cif3);
    free(cif4);
    free(cif5);
    free(cif6);
    free(cif7);
    free(cif8);
    free(cif9);
    fclose(input);
    fclose(afis_final);
    fclose(fisier_sursa);
    fclose(cifra0);
    fclose(cifra1);
    fclose(cifra2);
    fclose(cifra3);
    fclose(cifra4);
    fclose(cifra5);
    fclose(cifra6);
    fclose(cifra7);
    fclose(cifra8);
    fclose(cifra9);
    return 0;

}
