/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



package ifc.style;

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.style.Style</code>
* service properties :
* <ul>
*  <li><code> IsPhysical</code></li>
*  <li><code> FollowStyle</code></li>
*  <li><code> DisplayName</code></li>
*  <li><code> IsAutoUpdate</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.Style
*/
public class _Style extends MultiPropertyTest {

    public void _FollowStyle() {
        String style = (String)tEnv.getObjRelation("FollowStyle");
        if (style == null) style = "Heading 1";
        testProperty("FollowStyle", style, "Heading 2");
    }
    
}  // finish class _Style


