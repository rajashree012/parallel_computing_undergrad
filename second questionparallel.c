#include<stdio.h>
#include<stdlib.h>
struct linked_list
{
	int num;
	struct linked_list *next;
};
typedef struct linked_list node;
struct tourt
{
	node *head;
	int count;
	int cost;
};
typedef struct tourt tour_t;
tour_t *general;
void Depth_first_search(tour_t *tour);
int city_count(tour_t *tour);
int best_tour(tour_t *tour);
void update_best_tour(tour_t *tour);
int feasible(tour_t *tour,int city);
void add(tour_t *tour,int city);
void remove_last_city(tour_t *tour,int city);
int w[4][4]={{0,1,2,0},{0,0,4,0},{0,0,0,2},{3,0,0,0}};
main()
{
	int i;
	general=(tour_t *)malloc(sizeof(tour_t));
	general->cost=999;
	tour_t *tour=(tour_t *)malloc(sizeof(tour_t));
	tour->head=(node *)malloc(sizeof(node));
	tour->head->num=0;
	tour->head->next=NULL;
	tour->count=1;
	tour->cost=0;
	Depth_first_search(tour);
	printf("tour cost is %d",general->cost);
}
void Depth_first_search(tour_t *tour)
{
	int city,city1=0;
	int i=0;
	static int j=-1;
	node *temp=tour->head;
	if(city_count(tour)==4)
	{
		if(best_tour(tour))
		update_best_tour(tour);
		temp-tour->head;
		printf("\n");
		while(temp->next!=NULL)
		{
			printf("%d->",temp->num);
			temp=temp->next;
		}
		printf("%d->",temp->num);
	}
	else
	{
		for(i=0;i<4;i++)
		{
			temp=tour->head;
			while(temp->next!=NULL)
			{
				temp=temp->next;
			}
			city1=temp->num;
			if(w[city1][i]!=0)
			{
				city=i;
				if(feasible(tour,city))
				{
					add(tour,city);
					Depth_first_search(tour);
					remove_last_city(tour,city);
				}
			}
		}
	}
}
int city_count(tour_t *tour)
{
	return tour->count;
}
int best_tour(tour_t *tour)
{
	node *temp1=tour->head;
	int amount;
	while(temp1->next!=NULL)
	{
		temp1=temp1->next;
	}
	amount=tour->cost+w[temp1->num][0];
	if(general->cost>amount)
	{general->cost=amount;return 1;}
	else 
	return 0;
}
void update_best_tour(tour_t *tour)
{
	return;
}
int feasible(tour_t *tour,int city)
{
	node *temp=tour->head,*temp1=tour->head,*temp2=tour->head;
	int amount=0;
	while(temp->next!=NULL)
	{
		if(temp->num==city)
		{return 0;}
		temp=temp->next;
	}
	while(temp1->next!=NULL)
	{
		temp1=temp1->next;
	}
	amount=tour->cost+w[temp->num][city];
	if(amount>general->cost)
	return 0;
	else
	return 1;
}
void add(tour_t *tour,int city)
{
	node *temp=tour->head;
	while(temp->next!=NULL)
	{
		temp=temp->next;
	}
	temp->next=(node *)malloc(sizeof(node));
	temp->next->next==NULL;
	temp->next->num=city;
	tour->cost=tour->cost+w[temp->num][city];
	tour->count=tour->count+1;
}
void remove_last_city(tour_t *tour,int city)
{
	node *temp=tour->head;
	if(temp->next==NULL)
	return;
	while(temp->next->next!=NULL)
	{
		temp=temp->next;
	}
	temp->next=NULL;
	tour->cost=tour->cost-w[temp->num][city];
	tour->count=tour->count-1;
}
