#include "PathFinding.h"
#include "World.hpp"

#define ABS(x) (((x) < 0) ? -(x) : (x))

PathFinding::PathFinding(World world):
	m_start(0,0),
	m_goal(0,0),
	m_world(world)
{
	for(int i(0); i <= world.worldLength; i += m_world.step)
		for(int j(0); j <= world.worldWidth; j +=  m_world.step)
		{
			if(!world.checkObstacle(Vecteur(i,j)))
			{
				m_grille[j*m_world.worldSize + i] = Node(i, j, m_world);

				sf::CircleShape shape( m_world.step / 4);
				shape.setFillColor(sf::Color::Black);
				shape.setPosition(i,j);
				m_shapes.push_back(shape);
			}
		}
}


PathFinding::~PathFinding(void)
{
	//delete m_currentNode;
}

//Sert a lancer la recherche, fait les initialisations
void PathFinding::findPath(Vecteur start, Vecteur goal)
{
	m_start = start;
	m_goal = goal;

	
	//Si le d�part est au m�me endroit que l'arriv�e ou sur un obstacle
	if((m_start.x == m_goal.x && m_start.y == m_goal.y) || m_world.checkObstacle(Vecteur(m_goal.x, m_goal.y)))
	{
		//On met l'arriv�e dans la liste des positions
		m_resultPath.push_back(Vecteur(m_goal.x, m_goal.y));
		return;
	}
	
	//Clear
	m_openList.clear();
	m_closedList.clear();
	m_resultPath.clear();

	int startId = start.y*m_world.worldSize + start.x;
	int goalId = goal.y*m_world.worldSize + goal.x;

	//On initialise les noeuds de d�part et d'arriv�e
	m_currentNode = new Node(m_grille[startId]);
	m_goalNode = m_grille[goalId];

	//On rajoute la node de d�part � la liste des nodes disponibles
	m_closedList.push_back(*m_currentNode);

	//On check les nodes alentours
	this->checkNeighbourNode();
}

//Rajoute une node � la liste des nodes disponibles pour le chemin
void PathFinding::addToOpenList(float x, float y, float moveCost,  Node* parent)
{	
	int id = y * m_world.worldSize + x;
	std::unordered_map<int, Node>::const_iterator exist = m_grille.find(id);
	if(exist == m_grille.end())
		return;

	//On s'arr�te l� si les calculs ont d�j� �t� fait pour cette node
	std::vector<Node>::iterator it = m_openList.begin();
	for(; it != m_openList.end(); it++)
		if(id == (it)->m_id)
			return;
	std::vector<Node>::iterator itt = m_closedList.begin();
	for(; itt != m_closedList.end(); itt++)
		if(id == (itt)->m_id)
			return;

	m_grille[id].G = moveCost;
	m_grille[id].manHattanDistance(m_goalNode);
	m_grille[id].parent = parent;
	m_openList.push_back(m_grille[id]);
	
    sf::CircleShape shape( m_world.step / 4);
    shape.setFillColor(sf::Color::Green);
	shape.setPosition(x, y);
	m_shapes.push_back(shape);
}

//Lance les calculs sur les nodes alentours, v�rifie si l'on est arriv�
void PathFinding::checkNeighbourNode()
{
	//Si l'objectif n'est pas proche
	if(!m_currentNode->isClosed(m_goalNode))
	{
		this->addToOpenList(m_currentNode->m_x +m_world.step, m_currentNode->m_y, 1, m_currentNode);
		this->addToOpenList(m_currentNode->m_x, m_currentNode->m_y +m_world.step, 1, m_currentNode);
		this->addToOpenList(m_currentNode->m_x +m_world.step, m_currentNode->m_y +m_world.step, 1.4f, m_currentNode);
		this->addToOpenList(m_currentNode->m_x -m_world.step, m_currentNode->m_y -m_world.step, 1.4f, m_currentNode);
		this->addToOpenList(m_currentNode->m_x -m_world.step, m_currentNode->m_y, 1, m_currentNode);
		this->addToOpenList(m_currentNode->m_x -m_world.step, m_currentNode->m_y +m_world.step, 1.4f, m_currentNode);
		this->addToOpenList(m_currentNode->m_x +m_world.step, m_currentNode->m_y -m_world.step, 1.4f, m_currentNode);
		this->addToOpenList(m_currentNode->m_x, m_currentNode->m_y -m_world.step, 1, m_currentNode);

		//Si m_openList est vide, cela signifie que la position demand� n'est pas accessible
		if(m_openList.empty())
			return;
		else
		{
			//On regarde la prochaine node sur laquelle on va fixer nos recherches
			//On choisit la node dans m_openList avec le F minimum
			float Fvalue = 99999;
			std::vector<Node>::iterator eraseIt;
			std::vector<Node>::iterator it = m_openList.begin();
			for(; it != m_openList.end(); it++)
				if(Fvalue > (it)->getF())
				{
					Fvalue = (it)->getF();
					eraseIt = it;
				}
			m_currentNode = new Node(*eraseIt);
			//Et on la rajoute � la liste des nodes potentiellement sur le chemin final
			m_closedList.push_back(*m_currentNode);
			//On l'enl�ve de la liste des nodes disponibles
			m_openList.erase(eraseIt);

			sf::CircleShape shape( m_world.step / 4);
			shape.setFillColor(sf::Color::Red);
			shape.setPosition(m_currentNode->m_x, m_currentNode->m_y);
			m_shapes.push_back(shape);

			//On check les nodes alentours : r�cursif
			this->checkNeighbourNode();
		}
	}
	//Si on est proche de l'objectif
	else
	{
		//On r�cup�re le parent des nodes une � une pour remonter jusqu'au d�part
		m_goalNode.parent = m_currentNode;
		for(Node* getPath = new Node(m_goalNode); getPath != NULL; getPath = getPath->parent)
		{
			std::cout << getPath->m_x << " " << getPath->m_y << std::endl;
				m_resultPath.push_back(Vecteur(getPath->m_x, getPath->m_y));
				sf::CircleShape shape( m_world.step / 4);
				shape.setFillColor(sf::Color::Blue);
				shape.setPosition(getPath->m_x, getPath->m_y);
				m_shapes.push_back(shape);
		}
		delete m_currentNode;
		return;
	}
}

//Renvoie le chemin vers l'objectif
std::vector<Vecteur> PathFinding::getPath()
{
	return m_resultPath;
}

void PathFinding::draw(sf::RenderWindow &window)
{
	std::vector<sf::CircleShape>::iterator it = m_shapes.begin();
	for(; it != m_shapes.end(); it++)
		window.draw(*it);
}

bool PathFinding::lineOfSight(std::shared_ptr<Node> startNode, std::shared_ptr<Node> goalNode)
{
	int x = startNode->m_x;
	int y = startNode->m_y;
	int gx = goalNode->m_x;
	int gy = goalNode->m_y;

	int dx = gx - x;
	int dy = gy - y;
	int sx = 5;
	if(dx < 0)
	{
		sx = -sx;
		dx = - dx;
	}

	if(dy < 0)
		dy = - dy;

	float a, b;

	if(dx== 0 && dy ==0)
		return true;

	if(dy != 0)
		a = (gx - x)/(gy - y);
	else
		a = 0;
	b = y - a*x;
	
	std::cout << x << "," << y << " " << gx << "," << gy << std::endl;
		
	while(dy > 10 && dx > 10)
	{
		y = b + a*x;
		
		if(m_world.checkObstacle(Vecteur(x,y)))
		{
			std::cout << x << " "  << y << std::endl;
			return false;
		}
		dx = ABS(gx - x);
		dy = ABS(gy - y);
		x += sx;
	}
	/*
		*/
	
	return true;
	/*

	int x = startNode->m_x;
	int y = startNode->m_y;
	int gx = goalNode->m_x;
	int gy = goalNode->m_y;

	int dx = gx - x;
	int dy = gy - y;

	int sx = m_world.step;
	int sy = m_world.step;

	int f = 0;

	if(dx < 0)
	{
		sx = -sx;
		dx = - dx;
	}

	if(dy < 0)
	{
		sy = -sy;
		dy = - dy;

	if(dx >= dy)
		while(x != gx)
		{
			f += dy;
			if(f >= dx)
			{
				if(m_world.checkObstacle(Vecteur(x + (sx - 1)/2,y + (sy - 1)/2)))
					return false;
				y += sy;
				f -= dx;
			}
			if(f != 0 && m_world.checkObstacle(Vecteur(x + (sx - 1)/2, y + (sy - 1)/2)))
				return false;
			if(dy == 0 && m_world.checkObstacle(Vecteur(x + (sx - 1)/2,y)) && m_world.checkObstacle(Vecteur(x + (sx - 1)/2, y - 1)) )
				return false;
			x += sx;
		}
	else
		while(y != gy)
		{
			f += dx;
			if(f >= dy)
			{
				if(m_world.checkObstacle(Vecteur(x + (sx - 1)/2,y + (sy - 1)/2)))
					return false;
				x += sx;
				f -= dy;
			}
			if(f != 0 && m_world.checkObstacle(Vecteur(x + (sx - 1)/2, y + (sy - 1)/2)))
				return false;
			if(dx == 0 && m_world.checkObstacle(Vecteur(x,y + (sy - 1)/2)) && m_world.checkObstacle(Vecteur(x -1, y + (sy - 1)/2)) )
				return false;
			y += sy;
		}
		*/
		return true;
}