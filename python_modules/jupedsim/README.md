## Agent Distribution
* The Distribution allowes to randomly distribute agents inside a Polygon

* Agents will keep distance to other agents as well as to the polygon borders

* There are four methods to distribute agents:

	* distribute by density or number
	* distribute in circle segments by number or density
	
* Constraints
	* When distributing to two polygons, each polygon must be distributed once.
	* The polygons must not overlap
	* The polygons may touch each other if the added wall distance of the two distributions is equal or greater than the largest agent distance.

## Testing

* Test files are located in ```jpscore/python_modules/jupedsim/tests```

* to test pytest files you can call ```python -m pytest``` from ```jpscore/python_modules/jupedsim``` 

* to test the distribution functions visually call ```python -m streamlit run jupedsim/app.py``` from ```jpscore/python_modules/jupedsim```
