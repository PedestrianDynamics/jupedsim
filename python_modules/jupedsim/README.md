## Agent Distribution
* Constraints
	* When distributing to two polygons, each polygon must be distributed once.
	* The polygons must not overlap
	* The polygons may touch each other if the added wall distance of the two distributions is equal or greater than the largest agent distance.

## Testing

from ```jpscore/python_modules/jupedsim``` you can call ```python -m pytest```

to test the Distribution visually call ```python -m streamlit run jupedsim/app.py``` from ```jpscore/python_modules/jupedsim```
